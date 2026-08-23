#ifndef MONITORPLAYLISTSTATE_H
#define MONITORPLAYLISTSTATE_H

#include "models/playlistenums.h"
#include "models/queuemodel.h"
#include <QObject>

// Kompletny, w pełni niezależny stan planowania tapet dla JEDNEGO monitora

// Tryb/ustawienia trzymane są jako PlaylistEnums (nie int) — patrz
// playlistenums.h po uzasadnienie i mapowanie na stringi w JSON-ie.
class MonitorPlaylistState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PlaylistEnums::Mode currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(PlaylistEnums::OrderMode loginOrderMode READ loginOrderMode WRITE setLoginOrderMode NOTIFY loginOrderModeChanged)
    Q_PROPERTY(PlaylistEnums::OrderMode timerOrderMode READ timerOrderMode WRITE setTimerOrderMode NOTIFY timerOrderModeChanged)
    Q_PROPERTY(int timerIntervalValue READ timerIntervalValue WRITE setTimerIntervalValue NOTIFY timerIntervalValueChanged)
    Q_PROPERTY(PlaylistEnums::IntervalUnit timerIntervalUnit READ timerIntervalUnit WRITE setTimerIntervalUnit NOTIFY timerIntervalUnitChanged)

public:
    explicit MonitorPlaylistState(QObject* parent = nullptr)
        : QObject(parent)
        , m_model(new QueueModel(this))
    {
    }

    QueueModel* queueModel() const { return m_model; }

    PlaylistEnums::Mode currentMode() const { return m_currentMode; }
    void setCurrentMode(PlaylistEnums::Mode mode)
    {
        if (m_currentMode == mode)
            return;
        m_currentMode = mode;
        emit currentModeChanged();
    }

    PlaylistEnums::OrderMode loginOrderMode() const { return m_loginOrderMode; }
    void setLoginOrderMode(PlaylistEnums::OrderMode mode)
    {
        if (m_loginOrderMode == mode)
            return;
        m_loginOrderMode = mode;
        emit loginOrderModeChanged();
    }

    PlaylistEnums::OrderMode timerOrderMode() const { return m_timerOrderMode; }
    void setTimerOrderMode(PlaylistEnums::OrderMode mode)
    {
        if (m_timerOrderMode == mode)
            return;
        m_timerOrderMode = mode;
        emit timerOrderModeChanged();
    }

    int timerIntervalValue() const { return m_timerIntervalValue; }
    void setTimerIntervalValue(int value)
    {
        if (m_timerIntervalValue == value)
            return;
        m_timerIntervalValue = value;
        emit timerIntervalValueChanged();
    }

    PlaylistEnums::IntervalUnit timerIntervalUnit() const { return m_timerIntervalUnit; }
    void setTimerIntervalUnit(PlaylistEnums::IntervalUnit unit)
    {
        if (m_timerIntervalUnit == unit)
            return;
        m_timerIntervalUnit = unit;
        emit timerIntervalUnitChanged();
    }

signals:
    void currentModeChanged();
    void loginOrderModeChanged();
    void timerOrderModeChanged();
    void timerIntervalValueChanged();
    void timerIntervalUnitChanged();

private:
    QueueModel* m_model;

    PlaylistEnums::Mode m_currentMode = PlaylistEnums::Mode::TimeOfDay;
    PlaylistEnums::OrderMode m_loginOrderMode = PlaylistEnums::OrderMode::Random;
    PlaylistEnums::OrderMode m_timerOrderMode = PlaylistEnums::OrderMode::Random;
    int m_timerIntervalValue = 30;
    PlaylistEnums::IntervalUnit m_timerIntervalUnit = PlaylistEnums::IntervalUnit::Minutes;
};

#endif // MONITORPLAYLISTSTATE_H
