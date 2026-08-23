#ifndef QUEUEMODEL_H
#define QUEUEMODEL_H

#include "models/queueitem.h"
#include <QAbstractListModel>
#include <QObject>

class QueueModel : public QAbstractListModel {
    Q_OBJECT

    // Liczba elementów w kolejce, dostępna z dowolnego pliku QML
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        SourcePathRole,
        ExportedPathRole,
        NameRole,
        HueRole,
        BrightnessRole,
        SaturationRole,
        FlippedRole,
        LutPathRole,
        ScheduleStartMinRole,
        ScheduleEndMinRole,
        WeekdayMaskRole
    };

    explicit QueueModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_items.size(); }

    void addOrUpdate(const QueueItem& item); // po id
    void remove(const QString& id);
    void move(int from, int to);


    void clear();
    Q_INVOKABLE QString idAt(int row) const;

    // Time of the day
    Q_INVOKABLE void distributeTimeSlotsEvenly();
    // proposedBoundaryMin is raw/un-snapped — this method owns snapping (5 min)
    // and the minimum slot width (30 min).
    Q_INVOKABLE void moveTimeSlotDivider(int dividerIndex, qreal proposedBoundaryMin);
    Q_INVOKABLE int scheduleStartMinAt(int row) const;
    Q_INVOKABLE int scheduleEndMinAt(int row) const;

    // Day of week
    Q_INVOKABLE void distributeWeekdaysEvenly();
    Q_INVOKABLE void moveWeekdayDivider(int dividerIndex, qreal proposedBoundaryDay);
    Q_INVOKABLE int scheduleStartDayAt(int row) const; // decoded from weekdayMask
    Q_INVOKABLE int scheduleEndDayAt(int row) const;

signals:
    void countChanged();

private:
    // Linear scan by id, used by every id-keyed mutator below. m_items is
    // expected to stay small (a single playlist's worth of wallpapers), so
    // O(n) here is fine and keeps QueueItem::id as the only identity we
    // need to track (no separate id->row map to keep in sync).
    int findRow(const QString& id) const;

    QList<QueueItem> m_items;
};

#endif // QUEUEMODEL_H
