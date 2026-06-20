#include "galleryservice.h"
#include <QDirIterator>
#include <QUrl>
#include <QFileInfo>

QList<ImageItem> GalleryService::scanFolder(const QString &folderPath) const
{
    QList<ImageItem> results;
    QStringList filters = {"*.jpg", "*.jpeg", "*.png"};
    QDirIterator it(folderPath, filters, QDir::Files | QDir::NoDotAndDotDot);

    while (it.hasNext()) {
        it.next();
        QFileInfo fi = it.fileInfo();
        // results.append({ QUrl::fromLocalFile(fi.absoluteFilePath()).toString(), fi.fileName() });
        results.append({ "image://taptimage/" + fi.absoluteFilePath(), fi.fileName() });
    }
    return results;
}

// QList<ImageItem> GalleryService::scanFolder(const QString &folderPath, int offset, int limit) const{
//     QList<ImageItem> results;
//     QStringList filters = {"*.jpg", "*.jpeg", "*.png"};
//     QDirIterator it(folderPath, filters, QDir::Files | QDir::NoDotAndDotDot);

//     int current = 0;
//     while (it.hasNext()) {
//         it.next();
//         if (current < offset) {
//             current++;
//             continue;
//         }
//         if (results.count() >= limit)
//             break;

//         QFileInfo fileInfo = it.fileInfo();
//         ImageItem item;
//         item.url = QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toString();
//         item.name = fileInfo.fileName();
//         results.append(item);
//         current++;
//     }

//     return results;
// }
