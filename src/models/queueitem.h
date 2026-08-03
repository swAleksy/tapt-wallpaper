#ifndef QUEUEITEM_H
#define QUEUEITEM_H

#include <QString>
#include "models/editstate.h"

struct QueueItem {
    QString   id;            // QUuid::createUuid().toString()
    QString   sourcePath;    // ścieżka do ORYGINALNEGO, niezmienionego pliku
    QString   name;
    EditState edit;
    QString   exportedPath = "//ph_path";  // na razie zawsze puste — zarezerwowane pod przyszły render dla skryptu (nie używane teraz)
};

#endif // QUEUEITEM_H
