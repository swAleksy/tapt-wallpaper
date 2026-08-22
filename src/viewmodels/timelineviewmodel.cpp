#include "viewmodels/timelineviewmodel.h"
#include "models/editstate.h"
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QScreen>
#include <QUuid>
#include <qcoreapplication.h>

TimelineViewModel::TimelineViewModel(QObject *parent)
    : QObject(parent)
{
    QScreen* primary = QGuiApplication::primaryScreen();
    m_currentMonitorId = primary ? primary->name() : QStringLiteral("default");
    ensureMonitorState(m_currentMonitorId);
}

MonitorPlaylistState* TimelineViewModel::ensureMonitorState(const QString& id)
{
    auto it = m_monitorStates.find(id);
    if (it != m_monitorStates.end())
        return it.value();

    auto* state = new MonitorPlaylistState(this);
    m_monitorStates.insert(id, state);
    return state;
}

MonitorPlaylistState* TimelineViewModel::currentState() const
{
    MonitorPlaylistState* state = m_monitorStates.value(m_currentMonitorId);

    // Invariant: m_currentMonitorId always has a matching entry in
    // m_monitorStates, because the only two writers of m_currentMonitorId
    // (the ctor and setCurrentMonitorId()) both call ensureMonitorState()
    // first. Every other method on this class dereferences currentState()
    // without a null check, so if this ever fires, look at whichever code
    // path just changed m_currentMonitorId directly.
    Q_ASSERT(state);
    return state;
}

void TimelineViewModel::setCurrentMonitorId(const QString& id)
{
    if (id.isEmpty() || id == m_currentMonitorId)
        return;

    m_currentMonitorId = id;
    ensureMonitorState(id);

    // Jeden emit wystarczy: monitorState (Q_PROPERTY, READ currentState)
    // dzieli ten sam NOTIFY co currentMonitorId, więc QML powiązany np. z
    // `TimelineViewModel.monitorState.currentMode` automatycznie przełączy
    // subskrypcję na sygnały nowego obiektu MonitorPlaylistState. Wcześniej
    // trzeba tu było ręcznie disconnect/connect i re-emitować pięć
    // osobnych sygnałów (currentModeChanged, loginOrderModeChanged, ...) —
    // to zniknęło razem z properties-proxy w nagłówku.
    emit currentMonitorIdChanged();
}

QueueModel* TimelineViewModel::queueModel() const
{
    return currentState()->queueModel();
}

QString TimelineViewModel::addItem(
    const QString &sourcePath,
    const QString &name,
    qreal hue,
    qreal brightness,
    qreal saturation,
    bool flipped,
    const QString &lutPath)
{
    EditState state {
        hue,
        brightness,
        saturation,
        flipped,
        lutPath
    };

    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QueueItem newItem {id, sourcePath, name, state};

    queueModel()->addOrUpdate(newItem);
    return id;
}

void TimelineViewModel::applyEditState(const QString &id, const EditState &state)
{
    QueueModel* model = queueModel();

    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i);
        if (model->data(index, QueueModel::IdRole).toString() != id)
            continue;

        QueueItem updatedItem {
            id,
            model->data(index, QueueModel::SourcePathRole).toString(),
            model->data(index, QueueModel::NameRole).toString(),
            state
        };

        // addOrUpdate() nadpisuje CAŁĄ strukturę QueueItem, więc bez tego
        // edycja koloru/jasności (albo jej reset) po cichu zerowałaby
        // przypisania harmonogramu (Time of the day / Day of week).
        updatedItem.scheduleStartMin = model->data(index, QueueModel::ScheduleStartMinRole).toInt();
        updatedItem.scheduleEndMin = model->data(index, QueueModel::ScheduleEndMinRole).toInt();
        updatedItem.weekdayMask = model->data(index, QueueModel::WeekdayMaskRole).toInt();

        model->addOrUpdate(updatedItem);
        return;
    }
}

void TimelineViewModel::updateItem(
    const QString &id,
    qreal hue,
    qreal brightness,
    qreal saturation,
    bool flipped,
    const QString &lutPath)
{
    applyEditState(id, EditState { hue, brightness, saturation, flipped, lutPath });
}

void TimelineViewModel::resetItemToDefaults(const QString &id)
{
    applyEditState(id, EditState::identity());
}

void TimelineViewModel::removeItem(const QString &id)
{
    queueModel()->remove(id);
}

void TimelineViewModel::clearPlaylist()
{
    queueModel()->clear();
}

void TimelineViewModel::editItem(const QString &id)
{
    QueueModel* model = queueModel();

    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i);
        if (model->data(index, QueueModel::IdRole).toString() != id)
            continue;

        emit itemRequestedForEditing(
            id,
            model->data(index, QueueModel::SourcePathRole).toString(),
            model->data(index, QueueModel::NameRole).toString(),
            model->data(index, QueueModel::HueRole).toReal(),
            model->data(index, QueueModel::BrightnessRole).toReal(),
            model->data(index, QueueModel::SaturationRole).toReal(),
            model->data(index, QueueModel::FlippedRole).toBool(),
            model->data(index, QueueModel::LutPathRole).toString()
        );
        return;
    }
}

void TimelineViewModel::moveItem(int from, int to)
{
    queueModel()->move(from, to);
}

namespace {

// Serializuje JEDEN stan monitora (tryb + ustawienia wszystkich trybów +
// jego kolejka) do formatu identycznego z tym, co dawniej trafiało do
// całego pliku w wersji jednomonitorowej — tyle że teraz jedna taka
// sekcja na monitor, pod "monitors"[monitorId] w exportPlaylist().
// "mode"/"orderMode"/"intervalUnit" idą do JSON-a jako STRINGI
// (PlaylistEnums::toString()), nie liczby — patrz playlistenums.h.
QJsonObject serializeMonitorState(const MonitorPlaylistState* state)
{
    QJsonObject obj;
    obj["mode"] = PlaylistEnums::toString(state->currentMode());

    // Ustawienia wszystkich trybów zapisujemy zawsze (nie tylko aktywnego)
    // — tak jak w wersji jednomonitorowej: prościej i bezpieczniej dla
    // czytającego serwisu niż warunkowe pola; serwis i tak powinien
    // patrzeć na "mode", żeby wiedzieć, który blok jest w danej chwili
    // istotny DLA TEGO MONITORA.
    QJsonObject loginSettings;
    loginSettings["orderMode"] = PlaylistEnums::toString(state->loginOrderMode());
    obj["whenLoggingIn"] = loginSettings;

    QJsonObject timerSettings;
    timerSettings["orderMode"] = PlaylistEnums::toString(state->timerOrderMode());
    timerSettings["intervalValue"] = state->timerIntervalValue();
    timerSettings["intervalUnit"] = PlaylistEnums::toString(state->timerIntervalUnit());
    obj["onATimer"] = timerSettings;

    QJsonArray queue;
    const QueueModel* model = state->queueModel();
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i);

        QJsonObject item;
        item["id"] = model->data(index, QueueModel::IdRole).toString();
        item["order"] = i;
        item["sourcePath"] = model->data(index, QueueModel::SourcePathRole).toString();
        // Na razie zawsze placeholder, dopóki render finalnych tapet
        // (z uwzględnieniem edit) nie zostanie podłączony — patrz komentarz
        // przy QueueItem::exportedPath.
        item["exportedPath"] = model->data(index, QueueModel::ExportedPathRole).toString();
        item["name"] = model->data(index, QueueModel::NameRole).toString();

        QJsonObject edit;
        edit["hue"] = model->data(index, QueueModel::HueRole).toDouble();
        edit["brightness"] = model->data(index, QueueModel::BrightnessRole).toDouble();
        edit["saturation"] = model->data(index, QueueModel::SaturationRole).toDouble();
        edit["flipped"] = model->data(index, QueueModel::FlippedRole).toBool();
        edit["lutPath"] = model->data(index, QueueModel::LutPathRole).toString();
        item["edit"] = edit;

        item["scheduleStartMin"] = model->data(index, QueueModel::ScheduleStartMinRole).toInt();
        item["scheduleEndMin"] = model->data(index, QueueModel::ScheduleEndMinRole).toInt();
        item["weekdayMask"] = model->data(index, QueueModel::WeekdayMaskRole).toInt();

        queue.append(item);
    }
    obj["queue"] = queue;

    return obj;
}

} // namespace

bool TimelineViewModel::exportPlaylist(const QString &path) const
{
    // WSZYSTKIE monitory naraz, nie tylko aktualnie wybrany w UI — inaczej
    // zapis wykonany podczas edycji monitora A wymazałby z pliku wszystko,
    // co wcześniej zapisano dla monitora B (a każdy Save w TimelinePanel
    // dotyczy tylko tego, co widać na ekranie w danej chwili).
    QJsonObject monitors;
    for (auto it = m_monitorStates.constBegin(); it != m_monitorStates.constEnd(); ++it)
        monitors[it.key()] = serializeMonitorState(it.value());

    QJsonObject root;
    root["monitors"] = monitors;

    const QJsonDocument doc(root);

    // Katalog docelowy może jeszcze nie istnieć (np. pierwszy zapis do
    // AppDataLocation) — QSaveFile go nie utworzy samo.
    const QDir dir = QFileInfo(path).dir();
    if (!dir.exists() && !dir.mkpath("."))
        return false;

    // Zapis atomowy: QSaveFile pisze do pliku tymczasowego w tym samym
    // katalogu i podmienia go na docelowy dopiero przy commit() — serwis
    // obserwujący `path` przez inotify/QFileSystemWatcher nigdy nie zobaczy
    // częściowo zapisanej treści.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}

void TimelineViewModel::distributeTimeSlotsEvenly()
{
    queueModel()->distributeTimeSlotsEvenly();
}

void TimelineViewModel::moveTimeSlotDivider(int i, qreal m)
{
    queueModel()->moveTimeSlotDivider(i, m);
}

void TimelineViewModel::distributeWeekdaysEvenly()
{
    queueModel()->distributeWeekdaysEvenly();
}

void TimelineViewModel::moveWeekdayDivider(int i, qreal d)
{
    queueModel()->moveWeekdayDivider(i, d);
}

void TimelineViewModel::switchMode(int modeIndex)
{
    currentState()->setCurrentMode(static_cast<PlaylistEnums::Mode>(modeIndex));
}
