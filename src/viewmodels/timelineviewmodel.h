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

class TimelineViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Stabilny identyfikator aktualnie wybranego monitora (np. "eDP-1",
    // "HDMI-A-1" — patrz QScreen::name() / Qt.application.screens w QML).
    // Celowo NIE indeks: kolejność monitorów w systemie potrafi się zmienić
    // między sesjami (odłączenie/podłączenie kabla, inna kolejność
    // wykrycia), a mapowanie po indeksie mogłoby po restarcie przypisać
    // playlistę do złego ekranu.
    Q_PROPERTY(QString currentMonitorId READ currentMonitorId WRITE setCurrentMonitorId NOTIFY currentMonitorIdChanged)

    // Promowane z lokalnych property w TimelinePanel.qml: to jedyne miejsce
    // (C++), które zna cały stan potrzebny do eksportu playlisty — patrz
    // exportPlaylist().

    Q_PROPERTY(QueueModel* queueModel READ queueModel NOTIFY currentMonitorIdChanged)

    // Typy poniżej to PlaylistEnums (nie int) — patrz playlistenums.h.
    // Wartości liczbowe pod spodem są bez zmian względem wersji
    // jednomonitorowej (0/1/2/3 dla trybu, 0/1 dla order/interval), więc
    // istniejący QML porównujący np. `currentMode === 3` nadal działa bez
    // zmian — enum jest tu głównie po to, żeby C++ i JSON (patrz
    // exportPlaylist()) nie posługiwały się gołymi liczbami.
    Q_PROPERTY(PlaylistEnums::Mode currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(PlaylistEnums::OrderMode loginOrderMode READ loginOrderMode WRITE setLoginOrderMode NOTIFY loginOrderModeChanged)
    Q_PROPERTY(PlaylistEnums::OrderMode timerOrderMode READ timerOrderMode WRITE setTimerOrderMode NOTIFY timerOrderModeChanged)
    Q_PROPERTY(int timerIntervalValue READ timerIntervalValue WRITE setTimerIntervalValue NOTIFY timerIntervalValueChanged)
    Q_PROPERTY(PlaylistEnums::IntervalUnit timerIntervalUnit READ timerIntervalUnit WRITE setTimerIntervalUnit NOTIFY timerIntervalUnitChanged)
    Q_PROPERTY(int maxDayOfWeekItems READ maxDayOfWeekItems CONSTANT)

public:
    explicit TimelineViewModel(QObject *parent = nullptr);

    QString currentMonitorId() const { return m_currentMonitorId; }

    void setCurrentMonitorId(const QString& id);

    QueueModel* queueModel() const;

    PlaylistEnums::Mode currentMode() const;
    void setCurrentMode(PlaylistEnums::Mode mode);

    PlaylistEnums::OrderMode loginOrderMode() const;
    void setLoginOrderMode(PlaylistEnums::OrderMode mode);

    PlaylistEnums::OrderMode timerOrderMode() const;
    void setTimerOrderMode(PlaylistEnums::OrderMode mode);

    int timerIntervalValue() const;
    void setTimerIntervalValue(int value);

    PlaylistEnums::IntervalUnit timerIntervalUnit() const;
    void setTimerIntervalUnit(PlaylistEnums::IntervalUnit unit);

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

    Q_INVOKABLE void switchMode(int modeIndex);
    // Usuwa całą playlistę AKTUALNIE WYBRANEGO monitora naraz (
    Q_INVOKABLE void clearPlaylist();

    Q_INVOKABLE void moveItem(int from, int to);

    // Atomowo zrzuca pełny stan i kolejkę do JSON pod `path` (bezpieczne dla plikowych watcherów).
    Q_INVOKABLE bool exportPlaylist(const QString& path) const;

    Q_INVOKABLE void distributeTimeSlotsEvenly();
    Q_INVOKABLE void moveTimeSlotDivider(int dividerIndex, qreal proposedBoundaryMin);
    Q_INVOKABLE void distributeWeekdaysEvenly();
    Q_INVOKABLE void moveWeekdayDivider(int dividerIndex, qreal proposedBoundaryDay);

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

    void currentMonitorIdChanged();
    void currentModeChanged();
    void loginOrderModeChanged();
    void timerOrderModeChanged();
    void timerIntervalValueChanged();
    void timerIntervalUnitChanged();

private:
    // Applies `state` as the item's new EditState while preserving its
    // schedule fields (scheduleStartMin/EndMin, weekdayMask) — shared by
    // updateItem() and resetItemToDefaults(), since addOrUpdate()
    // overwrites the whole QueueItem and both callers need the same
    // find-preserve-rebuild dance. No-op if `id` isn't in the queue.
    void applyEditState(const QString& id, const EditState& state);

    void connectMonitorState(MonitorPlaylistState* state);
    // Zwraca istniejący stan dla danego monitora albo tworzy nowy (pusta
    // kolejka, domyślny tryb), jeśli to pierwsze odwołanie do tego ekranu.
    MonitorPlaylistState* ensureMonitorState(const QString& id);

    // Stan monitora wskazywanego aktualnie przez m_currentMonitorId. Zawsze
    // istnieje po konstrukcji (patrz ctor) i po każdym setCurrentMonitorId().
    MonitorPlaylistState* currentState() const;

    QString m_currentMonitorId;

    // Owning map: each MonitorPlaylistState* is `new`'d with `this` as its
    // QObject parent (see ensureMonitorState()), so Qt's parent-child
    // ownership frees them on TimelineViewModel destruction. The map
    // itself does not own them independently.
    QMap<QString, MonitorPlaylistState*> m_monitorStates;
};

#endif // TIMELINEVIEWMODEL_H
