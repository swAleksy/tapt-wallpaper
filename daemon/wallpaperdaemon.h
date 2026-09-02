#ifndef WALLPAPERDAEMON_H
#define WALLPAPERDAEMON_H

#include "models/monitorplayliststate.h"
#include "models/playlistenums.h"
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>

class QueueModel;

class WallpaperDaemon : public QObject
{
    Q_OBJECT
public:
    explicit WallpaperDaemon(QObject* parent = nullptr);

private slots:
    // QFileSystemWatcher::directoryChanged — watchujemy KATALOG, nie sam
    // plik: QSaveFile zapisuje przez rename() na nowy inode, a obserwacja
    // samego pliku gubi się po takim rename. Katalog nie znika.
    void onPlaylistDirChanged(const QString& path);

    void reloadPlaylist();   // debounced -> PlaylistIO::importPlaylist()
    void tick();              // per-monitor: co powinno być wyświetlone TERAZ

private:
    struct TimerCycleState {
        int currentIndex = -1;
        QDateTime lastAdvance;
    };

    static QString pickForTimeOfDay(QueueModel* model, int nowMinutes);
    static QString pickForDayOfWeek(QueueModel* model, int today);
    static QString pickForOnATimer(QueueModel* model, MonitorPlaylistState* state, TimerCycleState& cycle);

    void applyWallpaper(const QString& monitorId, const QString& imagePath);
    void applyLoginPick(const QString& monitorId, MonitorPlaylistState* state);
    int screenIndexForMonitor(const QString& monitorId) const;

    QString m_playlistPath;

    QFileSystemWatcher m_watcher;
    QTimer m_reloadDebounce;
    QTimer m_tickTimer;

    QMap<QString, MonitorPlaylistState*> m_monitorStates;
    QMap<QString, QString> m_currentlyShown;       // monitorId -> ostatnio zaaplikowana ścieżka
    QMap<QString, TimerCycleState> m_timerCycle;   // monitorId -> stan cyklu dla OnATimer
};

#endif // WALLPAPERDAEMON_H
