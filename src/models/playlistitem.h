#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QString>
#include "models/editstate.h"

struct PlaylistItem {
    QString   id;            // QUuid::createUuid().toString()
    QString   sourcePath;    // ścieżka do ORYGINALNEGO, niezmienionego pliku
    QString   name;
    EditState edit;
    QString   exportedPath;  // na razie zawsze puste — zarezerwowane pod przyszły render dla skryptu (nie używane teraz)
};

#endif // PLAYLISTITEM_H
