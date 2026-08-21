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
    void resetToDefaults(const QString& id); // edit = EditState::identity()
    void move(int from, int to);


    void clear();
    Q_INVOKABLE QString idAt(int row) const;

    // Time of the day: przypisanie/wyczyszczenie przedziału czasowego.
    // startMin == -1 (lub endMin == -1) czyści przypisanie.
    Q_INVOKABLE void setTimeSlot(const QString& id, int startMin, int endMin);

    // Day of week: ustawienie pełnej maski bitowej naraz
    // Nie wymusza wyłączności między elementami — użyj assignDay(), jeśli
    // dany dzień ma należeć maks. do jednego obrazu naraz.
    Q_INVOKABLE void setWeekdayMask(const QString& id, int mask);

    // Przypisuje dzień (0=Pon ... 6=Nd) do jednego, konkretnego obrazu i
    // JEDNOCZEŚNIE zdejmuje ten dzień z wszystkich pozostałych elementów —
    // gwarantuje, że dany dzień tygodnia ma co najwyżej jednego "właściciela".
    Q_INVOKABLE void assignDay(const QString& id, int day);
    // Zdejmuje przypisanie dnia z konkretnego obrazu (bez wpływu na inne).
    Q_INVOKABLE void unassignDay(const QString& id, int day);

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
    int findRow(const QString& id) const;

    QList<QueueItem> m_items;
};

#endif // QUEUEMODEL_H
