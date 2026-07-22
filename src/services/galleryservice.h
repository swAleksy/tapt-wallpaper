#ifndef GALLERYSERVICE_H
#define GALLERYSERVICE_H

#include "models/imageItem.h"
#include <QObject>

class GalleryService {
public:
    QList<ImageItem> scanFolder(const QString& folderPath) const;
};

#endif // GALLERYSERVICE_H
