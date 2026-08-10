#ifndef TIMELINEVIEWMODEL_H
#define TIMELINEVIEWMODEL_H

#include "models/queuemodel.h"
#include <QObject>
#include <qqml.h>

class TimelineViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QueueModel* queueModel READ queueModel CONSTANT)

    // Stan eksportu trzymany w C++, aby nie ginął po zamknięciu widoku w QML.
    // 0 = Time of the day, 1 = When logging in, 2 = On a timer, 3 = Day of week
    Q_PROPERTY(int currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)

    // Ustawienia trybu "When logging in": 0 = Random, 1 = Ordered
    Q_PROPERTY(int loginOrderMode READ loginOrderMode WRITE setLoginOrderMode NOTIFY loginOrderModeChanged)

    // Ustawienia trybu "On a timer"
    Q_PROPERTY(int timerOrderMode READ timerOrderMode WRITE setTimerOrderMode NOTIFY timerOrderModeChanged) // 0 = Random, 1 = Ordered
    Q_PROPERTY(int timerIntervalValue READ timerIntervalValue WRITE setTimerIntervalValue NOTIFY timerIntervalValueChanged)
    Q_PROPERTY(int timerIntervalUnit READ timerIntervalUnit WRITE setTimerIntervalUnit NOTIFY timerIntervalUnitChanged) // 0 = Minutes, 1 = Hours

    // Limit elementów dla trybu "Day of week" (7), wystawiony bez zależności od widoków QML.
    Q_PROPERTY(int maxDayOfWeekItems READ maxDayOfWeekItems CONSTANT)

public:
    explicit TimelineViewModel(QObject *parent = nullptr);

    QueueModel* queueModel() const { return m_model; }

    int currentMode() const { return m_currentMode; }
    void setCurrentMode(int mode);

    int loginOrderMode() const { return m_loginOrderMode; }
    void setLoginOrderMode(int mode);

    int timerOrderMode() const { return m_timerOrderMode; }
    void setTimerOrderMode(int mode);

    int timerIntervalValue() const { return m_timerIntervalValue; }
    void setTimerIntervalValue(int value);

    int timerIntervalUnit() const { return m_timerIntervalUnit; }
    void setTimerIntervalUnit(int unit);

    static constexpr int kMaxDayOfWeekItems = 7;
    int maxDayOfWeekItems() const { return kMaxDayOfWeekItems; }

    Q_INVOKABLE QString addItem(
        const QString& sourcePath,
        const QString& name,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath);

    Q_INVOKABLE void updateItem(
        const QString& id,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath);

    Q_INVOKABLE void resetItemToDefaults(const QString& id);
    Q_INVOKABLE void removeItem(const QString& id);
    Q_INVOKABLE void editItem(const QString& id);

    // Czyści całą kolejkę (potwierdzenie w QML).
    Q_INVOKABLE void clearPlaylist();

    Q_INVOKABLE void moveItem(int from, int to);

    // Atomowo zrzuca pełny stan i kolejkę do JSON pod `path` (bezpieczne dla plikowych watcherów).
    Q_INVOKABLE bool exportPlaylist(const QString& path) const;

signals:
    void itemRequestedForEditing(
        const QString& id,
        const QString& sourcePath,
        const QString& name,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath);

    void currentModeChanged();
    void loginOrderModeChanged();
    void timerOrderModeChanged();
    void timerIntervalValueChanged();
    void timerIntervalUnitChanged();

private:
    QueueModel* m_model;

    int m_currentMode = 0;
    int m_loginOrderMode = 0;
    int m_timerOrderMode = 0;
    int m_timerIntervalValue = 30;
    int m_timerIntervalUnit = 0;
};

#endif // TIMELINEVIEWMODEL_H
