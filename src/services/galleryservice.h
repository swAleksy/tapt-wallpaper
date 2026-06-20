#ifndef GALLERYSERVICE_H
#define GALLERYSERVICE_H

#include <QObject>
#include "../models/imageItem.h"

class GalleryService
{
public:
    QList<ImageItem> scanFolder(const QString &folderPath) const;
};

#endif // GALLERYSERVICE_H
