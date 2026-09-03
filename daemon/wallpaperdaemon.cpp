#include "wallpaperdaemon.h"
#include "services/playlistio.h"
#include "models/queuemodel.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QRandomGenerator>
#include <QScreen>
#include <QTime>

namespace {
constexpr int kTickIntervalMs = 30 * 1000;
constexpr int kReloadDebounceMs = 300;

// Prefer the rendered image (with hue/brightness/saturation/LUT/flip
// applied by the GUI's PlaylistItemRenderer) over the raw source. Falls
// back to sourcePath when exportedPath is empty or the file doesn't exist
// yet (e.g. daemon started before the user ever saved a playlist).
QString resolvedPath(const QueueModel* model, const QModelIndex& idx)
{
    const QString exportedPath = model->data(idx, QueueModel::ExportedPathRole).toString();
    if (!exportedPath.isEmpty() && QFileInfo::exists(exportedPath))
        return exportedPath;
    return model->data(idx, QueueModel::SourcePathRole).toString();
}
} // namespace

QString WallpaperDaemon::pickForTimeOfDay(QueueModel* model, int nowMinutes)
{
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex idx = model->index(i);
        const int start = model->data(idx, QueueModel::ScheduleStartMinRole).toInt();
        const int end = model->data(idx, QueueModel::ScheduleEndMinRole).toInt();
        if (nowMinutes >= start && nowMinutes < end)
            return resolvedPath(model, idx);
    }
    return {};
}

QString WallpaperDaemon::pickForDayOfWeek(QueueModel* model, int today /* 0=pon..6=nd */)
{
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex idx = model->index(i);
        const int mask = model->data(idx, QueueModel::WeekdayMaskRole).toInt();
        if (mask & (1 << today))
            return resolvedPath(model, idx);
    }
    return {};
}

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

    return resolvedPath(model, model->index(cycle.currentIndex));
}

WallpaperDaemon::WallpaperDaemon(QObject* parent)
    : QObject(parent)
    , m_playlistPath(PlaylistIO::defaultPlaylistPath())
{
    // Pierwsze wczytanie zanim spinamy watcher/timery — inaczej własny
    // start wyglądałby dla siebie samego jak "zmiana z zewnątrz".
    reloadPlaylist();

    // Wczytanie ostatniego indeksu dla WhenLoggingIn/Ordered ZANIM
    // applyLoginPick() go użyje — plik jest w tym samym katalogu co
    // playlist.json, więc QFileSystemWatcher go też zobaczy, ale to
    // nie szkodzi: reloadPlaylist() nie rusza m_loginIndex.
    loadLoginState();

    // Ensure the directory exists before watching — addPath fails silently
    // if it doesn't, leaving the daemon blind to future saves (first run).
    const QString watchDir = QFileInfo(m_playlistPath).absolutePath();
    QDir().mkpath(watchDir);
    m_watcher.addPath(watchDir);
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

    int index;
    if (state->loginOrderMode() == PlaylistEnums::OrderMode::Random) {
        index = QRandomGenerator::global()->bounded(count);
    } else {
        // Ordered: advance from the last-shown index (persisted across
        // daemon restarts in login_state.json). First ever login starts
        // at 0, then cycles 1, 2, ... count-1, 0, 1, ...
        const int last = m_loginIndex.value(monitorId, -1);
        index = (last + 1) % count;
    }

    m_loginIndex[monitorId] = index;
    saveLoginState();

    applyWallpaper(monitorId, resolvedPath(model, model->index(index)));
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
    {
        QStringList known;
        for (QScreen* s : QGuiApplication::screens())
            known << s->name();
        qWarning() << "WallpaperDaemon: brak ekranu dla monitorId" << monitorId
                   << "- aktualnie widoczne ekrany:" << known;
        return;
    }

    // Update immediately so a tick before the dbus call returns doesn't
    // re-apply the same image.
    m_currentlyShown[monitorId] = imagePath;

    // plasma-apply-wallpaperimage has no --screen option in Plasma 6, so
    // we call the plasmashell dbus interface directly. This is a session-
    // bus call (local, fast), so the brief synchronous block is fine for
    // a daemon that ticks every 30 s.
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.plasmashell"),
        QStringLiteral("/PlasmaShell"),
        QStringLiteral("org.kde.PlasmaShell"),
        QStringLiteral("setWallpaper"));

    QVariantMap params;
    params[QStringLiteral("Image")] = imagePath;

    msg << QStringLiteral("org.kde.image")
        << params
        << static_cast<uint>(screenIndex);

    const QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() == QDBusMessage::ErrorMessage)
        qWarning() << "WallpaperDaemon: setWallpaper failed for monitor" << monitorId
                   << ":" << reply.errorMessage();
}

// MARK: - WhenLoggingIn/Ordered persistent index

QString WallpaperDaemon::loginStatePath() const
{
    return QFileInfo(m_playlistPath).dir().absoluteFilePath(QStringLiteral("login_state.json"));
}

void WallpaperDaemon::loadLoginState()
{
    QFile file(loginStatePath());
    if (!file.open(QIODevice::ReadOnly))
        return; // first run — no state file yet

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return;

    const QJsonObject obj = doc.object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
        m_loginIndex.insert(it.key(), it.value().toInt());
}

void WallpaperDaemon::saveLoginState() const
{
    QJsonObject obj;
    for (auto it = m_loginIndex.constBegin(); it != m_loginIndex.constEnd(); ++it)
        obj[it.key()] = it.value();

    const QDir dir = QFileInfo(loginStatePath()).dir();
    if (!dir.exists())
        dir.mkpath(".");

    QFile file(loginStatePath());
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}
