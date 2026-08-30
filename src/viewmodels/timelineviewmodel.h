#ifndef TIMELINEVIEWMODEL_H
#define TIMELINEVIEWMODEL_H

#include "models/editstate.h"
#include "models/monitorplayliststate.h"
#include "models/playlistenums.h"
#include "models/queuemodel.h"
#include <QMap>
#include <QObject>
#include <QString>
#include <qqml.h>
#include <QVariant>

class TimelineViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Stable monitor identifier (e.g. "eDP-1", "HDMI-A-1"; see
    // QScreen::name() / Qt.application.screens in QML). Not an index:
    // monitor order can change between sessions, which would remap a
    // playlist to the wrong screen after a restart.
    Q_PROPERTY(QString currentMonitorId READ currentMonitorId WRITE setCurrentMonitorId NOTIFY currentMonitorIdChanged)

    Q_PROPERTY(QueueModel *queueModel READ queueModel NOTIFY currentMonitorIdChanged)

    // All mode state (current mode + settings for every mode) lives in one
    // object instead of separate proxy properties. QML binds directly to
    // it, e.g. `TimelineViewModel.monitorState.currentMode`, and re-binds
    // automatically when this points at a different MonitorPlaylistState
    // (i.e. on monitor switch), since MonitorPlaylistState has its own
    // change signals. Adding a new mode setting only requires a change in
    // MonitorPlaylistState, not here.
    Q_PROPERTY(MonitorPlaylistState *monitorState READ currentState NOTIFY currentMonitorIdChanged)

    Q_PROPERTY(int maxDayOfWeekItems READ maxDayOfWeekItems CONSTANT)

public:
    explicit TimelineViewModel(QObject *parent = nullptr);

    static constexpr int kMaxDayOfWeekItems = PlaylistEnums::kDaysInWeek;

    // Property accessors.
    QString currentMonitorId() const { return m_currentMonitorId; }
    void setCurrentMonitorId(const QString &id);
    QueueModel *queueModel() const;
    MonitorPlaylistState *currentState() const;
    int maxDayOfWeekItems() const { return kMaxDayOfWeekItems; }

    // Item CRUD.
    Q_INVOKABLE QString addItem(
        const QString &sourcePath,
        const QString &name,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString &lutPath);
    Q_INVOKABLE void updateItem(
        const QString &id,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString &lutPath);
    Q_INVOKABLE void resetItemToDefaults(const QString &id);
    Q_INVOKABLE void removeItem(const QString &id);
    Q_INVOKABLE void editItem(const QString &id);
    Q_INVOKABLE void moveItem(int from, int to);

    Q_INVOKABLE void setItemExportedPath(const QString &id, const QString &path);
    Q_INVOKABLE bool ensurePlaylistDirectory() const;
    Q_INVOKABLE QVariantList allMonitorQueues() const;


    // Playlist-wide operations.
    Q_INVOKABLE void switchMode(int modeIndex);
    Q_INVOKABLE void clearPlaylist();

    // Persistence.
    Q_INVOKABLE bool exportPlaylist(const QString &path) const;
    Q_INVOKABLE bool importPlaylist(const QString &path);

    // Schedule layout helpers.
    Q_INVOKABLE void distributeTimeSlotsEvenly();
    Q_INVOKABLE void moveTimeSlotDivider(int dividerIndex, qreal proposedBoundaryMin);
    Q_INVOKABLE void distributeWeekdaysEvenly();
    Q_INVOKABLE void moveWeekdayDivider(int dividerIndex, qreal proposedBoundaryDay);

    Q_INVOKABLE QString playlistFilePath() const;

signals:
    void itemRequestedForEditing(
        const QString &id,
        const QString &sourcePath,
        const QString &name,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString &lutPath);

    void currentMonitorIdChanged();

private:
    QueueItem snapshotItem(const QString &id) const;

    // Shared by updateItem() and resetItemToDefaults(): applies `state` as
    // the item's new EditState while preserving its schedule fields
    // (scheduleStartMin/EndMin, weekdayMask), since addOrUpdate() replaces
    // the whole QueueItem. No-op if `id` isn't in the queue.
    void applyEditState(const QString &id, const EditState &state);

    // Returns the state for `id`, creating an empty one on first access.
    MonitorPlaylistState *ensureMonitorState(const QString &id);

    QString m_currentMonitorId;

    // Each MonitorPlaylistState is owned via Qt parent-child (parented to
    // `this` in ensureMonitorState()); the map itself does not own them.
    QMap<QString, MonitorPlaylistState *> m_monitorStates;
};

#endif // TIMELINEVIEWMODEL_H
