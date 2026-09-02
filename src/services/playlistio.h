#ifndef PLAYLISTIO_H
#define PLAYLISTIO_H

#include <QMap>
#include <QString>

class MonitorPlaylistState;
class QObject;

namespace PlaylistIO {

QString defaultPlaylistPath();

// Zapisuje wszystkie podane monitory do pliku JSON.
bool exportPlaylist(const QString &path, const QMap<QString, MonitorPlaylistState*> &monitorStates);

// Odczytuje plik JSON i aktualizuje mapę.
// Jeśli w pliku są nowe monitory (brakujące w mapie), zostaną one utworzone
// z podanym obiektem `parentForNew` (żeby Qt zarządzało ich pamięcią).
bool importPlaylist(const QString &path, QMap<QString, MonitorPlaylistState*> &monitorStates, QObject *parentForNew = nullptr);

} // namespace PlaylistIO

#endif // PLAYLISTIO_H
