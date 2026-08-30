#include "viewmodels/timelineviewmodel.h"
#include "models/editstate.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QScreen>
#include <QUuid>
#include <qcoreapplication.h>
#include <QStandardPaths>

namespace {

// Reads every field for `id` at `index` in `model` into a QueueItem.
// Shared by snapshotItem() (current monitor only) and
// setItemExportedPath() (searches every monitor).
QueueItem snapshotRow(const QueueModel *model, const QModelIndex &index, const QString &id)
{
    QueueItem item;
    item.id = id;
    item.sourcePath = model->data(index, QueueModel::SourcePathRole).toString();
    item.name = model->data(index, QueueModel::NameRole).toString();
    item.exportedPath = model->data(index, QueueModel::ExportedPathRole).toString();
    item.edit.hue = model->data(index, QueueModel::HueRole).toReal();
    item.edit.brightness = model->data(index, QueueModel::BrightnessRole).toReal();
    item.edit.saturation = model->data(index, QueueModel::SaturationRole).toReal();
    item.edit.flipped = model->data(index, QueueModel::FlippedRole).toBool();
    item.edit.lutPath = model->data(index, QueueModel::LutPathRole).toString();
    item.scheduleStartMin = model->data(index, QueueModel::ScheduleStartMinRole).toInt();
    item.scheduleEndMin = model->data(index, QueueModel::ScheduleEndMinRole).toInt();
    item.weekdayMask = model->data(index, QueueModel::WeekdayMaskRole).toInt();
    return item;
}

} // namespace

TimelineViewModel::TimelineViewModel(QObject *parent)
    : QObject(parent)
{
    QScreen *primary = QGuiApplication::primaryScreen();
    m_currentMonitorId = primary ? primary->name() : QStringLiteral("default");
    ensureMonitorState(m_currentMonitorId);

    importPlaylist(playlistFilePath());
}

// MARK: - Property accessors

void TimelineViewModel::setCurrentMonitorId(const QString &id)
{
    if (id.isEmpty() || id == m_currentMonitorId)
        return;

    m_currentMonitorId = id;
    ensureMonitorState(id);

    // monitorState shares this NOTIFY, so QML bound to e.g.
    // `TimelineViewModel.monitorState.currentMode` re-subscribes to the
    // new MonitorPlaylistState automatically.
    emit currentMonitorIdChanged();
}

QueueModel *TimelineViewModel::queueModel() const
{
    return currentState()->queueModel();
}

MonitorPlaylistState *TimelineViewModel::currentState() const
{
    MonitorPlaylistState *state = m_monitorStates.value(m_currentMonitorId);

    // Invariant: every writer of m_currentMonitorId (the ctor and
    // setCurrentMonitorId()) calls ensureMonitorState() first, so a lookup
    // miss here means some code path changed m_currentMonitorId directly.
    Q_ASSERT(state);
    return state;
}

// MARK: - Item CRUD

QString TimelineViewModel::addItem(
    const QString &sourcePath,
    const QString &name,
    qreal hue,
    qreal brightness,
    qreal saturation,
    bool flipped,
    const QString &lutPath)
{
    EditState state { hue, brightness, saturation, flipped, lutPath };

    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QueueItem newItem { id, sourcePath, name, state };

    queueModel()->addOrUpdate(newItem);
    return id;
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

void TimelineViewModel::editItem(const QString &id)
{
    QueueModel *model = queueModel();

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
            model->data(index, QueueModel::LutPathRole).toString());
        return;
    }
}

void TimelineViewModel::moveItem(int from, int to)
{
    queueModel()->move(from, to);
}

// MARK: - QML-driven wallpaper rendering (see PlaylistItemRenderer.qml)

QVariantList TimelineViewModel::allMonitorQueues() const
{
    QVariantList result;
    for (auto it = m_monitorStates.constBegin(); it != m_monitorStates.constEnd(); ++it) {
        QVariantMap entry;
        entry["monitorId"] = it.key();
        entry["queueModel"] = QVariant::fromValue(it.value()->queueModel());
        result.append(entry);
    }
    return result;
}

bool TimelineViewModel::ensurePlaylistDirectory() const
{
    const QDir dir = QFileInfo(playlistFilePath()).dir();
    return dir.exists() || dir.mkpath(".");
}

void TimelineViewModel::setItemExportedPath(const QString &id, const QString &path)
{
    for (auto it = m_monitorStates.constBegin(); it != m_monitorStates.constEnd(); ++it) {
        QueueModel *model = it.value()->queueModel();
        for (int i = 0; i < model->rowCount(); ++i) {
            const QModelIndex index = model->index(i);
            if (model->data(index, QueueModel::IdRole).toString() != id)
                continue;

            QueueItem item = snapshotRow(model, index, id);
            item.exportedPath = path;
            model->addOrUpdate(item);
            return;
        }
    }
}

QueueItem TimelineViewModel::snapshotItem(const QString &id) const
{
    QueueModel *model = queueModel();
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i);
        if (model->data(index, QueueModel::IdRole).toString() == id)
            return snapshotRow(model, index, id);
    }
    return QueueItem {};
}

void TimelineViewModel::applyEditState(const QString &id, const EditState &state)
{
    QueueItem item = snapshotItem(id);
    if (item.id.isEmpty())
        return;

    item.edit = state;
    queueModel()->addOrUpdate(item);
}

// MARK: - Playlist-wide operations

void TimelineViewModel::switchMode(int modeIndex)
{
    currentState()->setCurrentMode(static_cast<PlaylistEnums::Mode>(modeIndex));
}

void TimelineViewModel::clearPlaylist()
{
    queueModel()->clear();
}

// MARK: - Persistence

namespace {

// Serializes one monitor's state (mode + settings for every mode + queue)
// under "monitors"[monitorId] in exportPlaylist(). Enum fields are written
// as strings via PlaylistEnums::toString(), not raw numbers.
QJsonObject serializeMonitorState(const MonitorPlaylistState *state)
{
    QJsonObject obj;
    obj["mode"] = PlaylistEnums::toString(state->currentMode());

    // Settings for every mode are written unconditionally, not just the
    // active one; a reading service should look at "mode" to know which
    // block currently applies.
    QJsonObject loginSettings;
    loginSettings["orderMode"] = PlaylistEnums::toString(state->loginOrderMode());
    obj["whenLoggingIn"] = loginSettings;

    QJsonObject timerSettings;
    timerSettings["orderMode"] = PlaylistEnums::toString(state->timerOrderMode());
    timerSettings["intervalValue"] = state->timerIntervalValue();
    timerSettings["intervalUnit"] = PlaylistEnums::toString(state->timerIntervalUnit());
    obj["onATimer"] = timerSettings;

    QJsonArray queue;
    const QueueModel *model = state->queueModel();
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i);

        QJsonObject item;
        item["id"] = model->data(index, QueueModel::IdRole).toString();
        item["order"] = i;
        item["sourcePath"] = model->data(index, QueueModel::SourcePathRole).toString();
        // Placeholder until final wallpaper rendering (with edits baked
        // in) is wired up; see QueueItem::exportedPath.
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

// Intermediate structs for two-pass import validation: pass one parses and
// validates the whole file into these, pass two applies them to real state
// (see importPlaylist()).
struct ParsedQueueItem {
    QString id;
    QString sourcePath;
    QString exportedPath;
    QString name;
    EditState edit;
    int scheduleStartMin = 0;
    int scheduleEndMin = 0;
    int weekdayMask = 0;
};

struct ParsedMonitorState {
    PlaylistEnums::Mode mode = PlaylistEnums::Mode::TimeOfDay;
    PlaylistEnums::OrderMode loginOrderMode = PlaylistEnums::OrderMode::Random;
    PlaylistEnums::OrderMode timerOrderMode = PlaylistEnums::OrderMode::Random;
    int timerIntervalValue = 30;
    PlaylistEnums::IntervalUnit timerIntervalUnit = PlaylistEnums::IntervalUnit::Minutes;
    QList<ParsedQueueItem> items;
};

// Inverse of serializeMonitorState(). Returns false on the first mismatched
// field (missing field, wrong JSON type, unrecognized enum string) rather
// than guessing defaults, since a silent fallback would be worse than
// rejecting the whole file during config import.
bool parseMonitorState(const QJsonObject &obj, ParsedMonitorState &out)
{
    if (!obj["mode"].isString() || !PlaylistEnums::fromString(obj["mode"].toString(), out.mode))
        return false;

    if (!obj["whenLoggingIn"].isObject())
        return false;
    const QJsonObject login = obj["whenLoggingIn"].toObject();
    if (!login["orderMode"].isString() || !PlaylistEnums::fromString(login["orderMode"].toString(), out.loginOrderMode))
        return false;

    if (!obj["onATimer"].isObject())
        return false;
    const QJsonObject timer = obj["onATimer"].toObject();
    if (!timer["orderMode"].isString() || !PlaylistEnums::fromString(timer["orderMode"].toString(), out.timerOrderMode))
        return false;
    if (!timer["intervalValue"].isDouble())
        return false;
    out.timerIntervalValue = timer["intervalValue"].toInt();
    if (!timer["intervalUnit"].isString() || !PlaylistEnums::fromString(timer["intervalUnit"].toString(), out.timerIntervalUnit))
        return false;

    if (!obj["queue"].isArray())
        return false;

    for (const QJsonValue &v : obj["queue"].toArray()) {
        if (!v.isObject())
            return false;
        const QJsonObject item = v.toObject();

        if (!item["id"].isString() || item["id"].toString().isEmpty())
            return false;
        if (!item["edit"].isObject())
            return false;
        const QJsonObject edit = item["edit"].toObject();

        ParsedQueueItem qi;
        qi.id = item["id"].toString();
        qi.sourcePath = item["sourcePath"].toString();
        qi.exportedPath = item["exportedPath"].toString();
        qi.name = item["name"].toString();
        qi.edit.hue = edit["hue"].toDouble();
        qi.edit.brightness = edit["brightness"].toDouble();
        qi.edit.saturation = edit["saturation"].toDouble();
        qi.edit.flipped = edit["flipped"].toBool();
        qi.edit.lutPath = edit["lutPath"].toString();
        qi.scheduleStartMin = item["scheduleStartMin"].toInt();
        qi.scheduleEndMin = item["scheduleEndMin"].toInt();
        qi.weekdayMask = item["weekdayMask"].toInt();

        out.items.append(qi);
    }

    return true;
}

} // namespace

bool TimelineViewModel::exportPlaylist(const QString &path) const
{
    // All monitors at once, not just the one currently shown in the UI —
    // otherwise saving while editing monitor A would erase whatever was
    // previously saved for monitor B.
    QJsonObject monitors;
    for (auto it = m_monitorStates.constBegin(); it != m_monitorStates.constEnd(); ++it)
        monitors[it.key()] = serializeMonitorState(it.value());

    QJsonObject root;
    root["monitors"] = monitors;

    const QJsonDocument doc(root);

    // The target directory may not exist yet (e.g. first write to
    // AppDataLocation); QSaveFile won't create it.
    const QDir dir = QFileInfo(path).dir();
    if (!dir.exists() && !dir.mkpath("."))
        return false;

    // QSaveFile writes to a temp file and swaps it in on commit(), so a
    // watcher on `path` never sees a partially written file.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}

bool TimelineViewModel::importPlaylist(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    if (!doc.object()["monitors"].isObject())
        return false;
    const QJsonObject monitors = doc.object()["monitors"].toObject();

    // Pass 1: parse and validate into a local map without touching
    // m_monitorStates yet.
    QMap<QString, ParsedMonitorState> parsed;
    for (auto it = monitors.constBegin(); it != monitors.constEnd(); ++it) {
        if (!it.value().isObject())
            return false;

        ParsedMonitorState state;
        if (!parseMonitorState(it.value().toObject(), state))
            return false;

        parsed.insert(it.key(), state);
    }

    // Pass 2: data is already validated, so these assignments shouldn't fail.
    for (auto it = parsed.constBegin(); it != parsed.constEnd(); ++it) {
        MonitorPlaylistState *monitorState = ensureMonitorState(it.key());
        const ParsedMonitorState &state = it.value();

        monitorState->setCurrentMode(state.mode);
        monitorState->setLoginOrderMode(state.loginOrderMode);
        monitorState->setTimerOrderMode(state.timerOrderMode);
        monitorState->setTimerIntervalValue(state.timerIntervalValue);
        monitorState->setTimerIntervalUnit(state.timerIntervalUnit);

        QueueModel *model = monitorState->queueModel();
        model->clear();
        for (const ParsedQueueItem &qi : state.items) {
            QueueItem item { qi.id, qi.sourcePath, qi.name, qi.edit };
            item.exportedPath = qi.exportedPath;
            item.scheduleStartMin = qi.scheduleStartMin;
            item.scheduleEndMin = qi.scheduleEndMin;
            item.weekdayMask = qi.weekdayMask;
            model->addOrUpdate(item);
        }
    }

    return true;
}

// MARK: - Schedule layout helpers

void TimelineViewModel::distributeTimeSlotsEvenly()
{
    queueModel()->distributeTimeSlotsEvenly();
}

void TimelineViewModel::moveTimeSlotDivider(int dividerIndex, qreal proposedBoundaryMin)
{
    queueModel()->moveTimeSlotDivider(dividerIndex, proposedBoundaryMin);
}

void TimelineViewModel::distributeWeekdaysEvenly()
{
    queueModel()->distributeWeekdaysEvenly();
}

void TimelineViewModel::moveWeekdayDivider(int dividerIndex, qreal proposedBoundaryDay)
{
    queueModel()->moveWeekdayDivider(dividerIndex, proposedBoundaryDay);
}

QString TimelineViewModel::playlistFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + QStringLiteral("/playlist.json");
}

// MARK: - Internal helpers

MonitorPlaylistState *TimelineViewModel::ensureMonitorState(const QString &id)
{
    auto it = m_monitorStates.find(id);
    if (it != m_monitorStates.end())
        return it.value();

    auto *state = new MonitorPlaylistState(this);
    m_monitorStates.insert(id, state);
    return state;
}
