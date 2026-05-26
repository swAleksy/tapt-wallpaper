#ifndef GALLERYSERVICE_H
#define GALLERYSERVICE_H

#include <QObject>
#include "../models/imageItem.h"

class GalleryService
{
public:
    QList<ImageItem> scanFolder(const QString &folderPath, int offset = 0, int limit = 50) const;
};

#endif // GALLERYSERVICE_H
