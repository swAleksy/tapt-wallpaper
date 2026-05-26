#include "galleryservice.h"
#include <QDirIterator>
#include <QUrl>
#include <QFileInfo>


QList<ImageItem> GalleryService::scanFolder(const QString &folderPath, int offset, int limit) const{
    QList<ImageItem> results;
    QStringList filters = {"*.jpg", "*.jpeg", "*.png"};
    QDirIterator it(folderPath, filters, QDir::Files | QDir::NoDotAndDotDot);

    int current = 0;
    while (it.hasNext()) {
        it.next();
        if (current < offset) { 
            current++; 
            continue; 
        }      
        if (results.count() >= limit) 
            break;     

        QFileInfo fileInfo = it.fileInfo();
        ImageItem item;
        item.url = QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toString();
        item.name = fileInfo.fileName();
        results.append(item);
        current++; 
    }

    return results;
}