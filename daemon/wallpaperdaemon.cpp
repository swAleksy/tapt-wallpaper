#include "wallpaperdaemon.h"
#include "services/playlistio.h"
#include "models/queuemodel.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QGuiApplication>
#include <QProcess>
#include <QRandomGenerator>
#include <QScreen>
#include <QTime>

namespace {
constexpr int kTickIntervalMs = 30 * 1000;
constexpr int kReloadDebounceMs = 300;

QString pickForTimeOfDay(QueueModel* model, int nowMinutes)
{
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex idx = model->index(i);
        const int start = model->data(idx, QueueModel::ScheduleStartMinRole).toInt();
        const int end = model->data(idx, QueueModel::ScheduleEndMinRole).toInt();
        if (nowMinutes >= start && nowMinutes < end)
            return model->data(idx, QueueModel::SourcePathRole).toString();
    }
    return {};
}

QString pickForDayOfWeek(QueueModel* model, int today /* 0=pon..6=nd */)
{
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex idx = model->index(i);
        const int mask = model->data(idx, QueueModel::WeekdayMaskRole).toInt();
        if (mask & (1 << today))
            return model->data(idx, QueueModel::SourcePathRole).toString();
    }
    return {};
}

} // namespace

// Metoda klasy - ma dostęp do prywatnego TimerCycleState.
QString WallpaperDaemon::pickForOnATimer(QueueModel* model, MonitorPlaylistState* state,
                                          TimerCycleState& cycle)
{
    const int count = model->rowCount();
    const int intervalMs = state->timerIntervalUnit() == PlaylistEnums::IntervalUnit::Hours
        ? state->timerIntervalValue() * 3600 * 1000
        : state->timerIntervalValue() * 60 * 1000;

    const bool due = cycle.currentIndex < 0 || !cycle.lastAdvance.isValid()
        || cycle.lastAdvance.msecsTo(QDateTime::currentDateTime()) >= intervalMs;

    if (due) {
        cycle.currentIndex = state->timerOrderMode() == PlaylistEnums::OrderMode::Random
            ? QRandomGenerator::global()->bounded(count)
            : (cycle.currentIndex + 1) % count;
        cycle.lastAdvance = QDateTime::currentDateTime();
    }

    return model->data(model->index(cycle.currentIndex), QueueModel::SourcePathRole).toString();
}

WallpaperDaemon::WallpaperDaemon(QObject* parent)
    : QObject(parent)
    , m_playlistPath(PlaylistIO::defaultPlaylistPath())
{
    // Pierwsze wczytanie zanim spinamy watcher/timery — inaczej własny
    // start wyglądałby dla siebie samego jak "zmiana z zewnątrz".
    reloadPlaylist();

    m_watcher.addPath(QFileInfo(m_playlistPath).absolutePath());
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &WallpaperDaemon::onPlaylistDirChanged);

    m_reloadDebounce.setSingleShot(true);
    connect(&m_reloadDebounce, &QTimer::timeout, this, &WallpaperDaemon::reloadPlaylist);

    connect(&m_tickTimer, &QTimer::timeout, this, &WallpaperDaemon::tick);
    m_tickTimer.start(kTickIntervalMs);

    // WhenLoggingIn dostaje wybór RAZ, tutaj. Kolejne reloadPlaylist() go
    // nie powtarzają — patrz komentarz przy tick()'s case WhenLoggingIn.
    for (auto it = m_monitorStates.constBegin(); it != m_monitorStates.constEnd(); ++it) {
        if (it.value()->currentMode() == PlaylistEnums::Mode::WhenLoggingIn)
            applyLoginPick(it.key(), it.value());
    }

    tick(); // od razu, nie czekaj na pierwszy strzał tickTimera
}

void WallpaperDaemon::onPlaylistDirChanged(const QString&)
{
    m_reloadDebounce.start(kReloadDebounceMs);
}

void WallpaperDaemon::reloadPlaylist()
{
    if (!PlaylistIO::importPlaylist(m_playlistPath, m_monitorStates, this)) {
        qWarning() << "WallpaperDaemon: failed to load playlist" << m_playlistPath;
        return; // normalne przy pierwszym uruchomieniu, zanim GUI cokolwiek zapisze
    }

    // Indeks cyklu OnATimer może wskazywać poza nowy (skrócony) rozmiar
    // kolejki po edycji w GUI — przytnij, żeby tick() nie czytał spoza zakresu.
    for (auto it = m_monitorStates.constBegin(); it != m_monitorStates.constEnd(); ++it) {
        const int count = it.value()->queueModel()->rowCount();
        auto cycleIt = m_timerCycle.find(it.key());
        if (cycleIt != m_timerCycle.end() && cycleIt->currentIndex >= count)
            cycleIt->currentIndex = count > 0 ? count - 1 : -1;
    }

    tick();
}

void WallpaperDaemon::tick()
{
    const QTime now = QTime::currentTime();
    const int nowMinutes = now.hour() * 60 + now.minute();
    const int today = QDate::currentDate().dayOfWeek() - 1; // QDate: 1=pon..7=nd

    for (auto it = m_monitorStates.constBegin(); it != m_monitorStates.constEnd(); ++it) {
        const QString& monitorId = it.key();
        MonitorPlaylistState* state = it.value();
        QueueModel* model = state->queueModel();

        if (model->rowCount() == 0)
            continue;

        QString chosenPath;
        switch (state->currentMode()) {
        case PlaylistEnums::Mode::TimeOfDay:
            chosenPath = pickForTimeOfDay(model, nowMinutes);
            break;
        case PlaylistEnums::Mode::DayOfWeek:
            chosenPath = pickForDayOfWeek(model, today);
            break;
        case PlaylistEnums::Mode::OnATimer:
            chosenPath = pickForOnATimer(model, state, m_timerCycle[monitorId]);
            break;
        case PlaylistEnums::Mode::WhenLoggingIn:
            continue; // wybrane raz w applyLoginPick(), tick() tego nie rusza
        }

        if (!chosenPath.isEmpty() && chosenPath != m_currentlyShown.value(monitorId))
            applyWallpaper(monitorId, chosenPath);
    }
}

void WallpaperDaemon::applyLoginPick(const QString& monitorId, MonitorPlaylistState* state)
{
    QueueModel* model = state->queueModel();
    const int count = model->rowCount();
    if (count == 0)
        return;

    const int index = state->loginOrderMode() == PlaylistEnums::OrderMode::Random
        ? QRandomGenerator::global()->bounded(count)
        : 0;

    applyWallpaper(monitorId, model->data(model->index(index), QueueModel::SourcePathRole).toString());
}

int WallpaperDaemon::screenIndexForMonitor(const QString& monitorId) const
{
    const auto screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        if (screens[i]->name() == monitorId)
            return i;
    }
    return -1; // monitor obecnie niepodłączony — apply pomija cicho
}

void WallpaperDaemon::applyWallpaper(const QString& monitorId, const QString& imagePath)
{
    const int screenIndex = screenIndexForMonitor(monitorId);
    if (screenIndex < 0)
        return;

    // TODO: exportedPath (finalny render z hue/brightness/saturation/LUT)
    // nie jest jeszcze podłączony po stronie GUI (patrz komentarz przy
    // exportedPath w PlaylistIO::serializeMonitorState) — na razie daemon
    // zawsze aplikuje surowy sourcePath, bez uwzględnienia edycji.
    const int exitCode = QProcess::execute(
        QStringLiteral("plasma-apply-wallpaperimage"),
        {QStringLiteral("--screen"), QString::number(screenIndex), imagePath});

    if (exitCode != 0) {
        qWarning() << "WallpaperDaemon: plasma-apply-wallpaperimage zwróciło" << exitCode
                   << "dla monitora" << monitorId;
        return;
    }

    m_currentlyShown[monitorId] = imagePath;
}
