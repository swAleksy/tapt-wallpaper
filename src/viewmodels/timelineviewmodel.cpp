#include "viewmodels/timelineviewmodel.h"
#include "models/editstate.h"
#include "services/playlistio.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
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

    importPlaylist(PlaylistIO::defaultPlaylistPath());
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
    const QDir dir = QFileInfo(PlaylistIO::defaultPlaylistPath()).dir();
    return dir.exists() || dir.mkpath(".");
}

void TimelineViewModel::cleanExportDirectory() const
{
    // Deletes all .png files in the playlist directory before a fresh
    // export. Rendered images are named <uuid>.png; playlist.json and
    // login_state.json are left untouched.
    const QDir dir = QFileInfo(PlaylistIO::defaultPlaylistPath()).dir();
    const QStringList pngs = dir.entryList({QStringLiteral("*.png")}, QDir::Files);
    for (const QString &name : pngs)
        QFile::remove(dir.absoluteFilePath(name));
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

bool TimelineViewModel::exportPlaylist(const QString &path) const
{
    return PlaylistIO::exportPlaylist(path, m_monitorStates);
}

bool TimelineViewModel::importPlaylist(const QString &path)
{
    // Przekazujemy 'this', żeby ewentualne nowe monitory załadowane
    // z pliku były przypięte w drzewie własności QObject do TimelineViewModel.
    return PlaylistIO::importPlaylist(path, m_monitorStates, this);
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
    return PlaylistIO::defaultPlaylistPath();
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
