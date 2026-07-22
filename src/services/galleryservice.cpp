#include "services/galleryservice.h"
#include <QDirIterator>
#include <QFileInfo>
#include <QUrl>

QList<ImageItem> GalleryService::scanFolder(const QString& folderPath) const
{
    QList<ImageItem> results;
    QStringList filters = { "*.jpg", "*.jpeg", "*.png" };
    QDirIterator it(folderPath, filters, QDir::Files | QDir::NoDotAndDotDot);

    while (it.hasNext()) {
        it.next();
        QFileInfo fi = it.fileInfo();
        // results.append({ QUrl::fromLocalFile(fi.absoluteFilePath()).toString(), fi.fileName() });
        results.append({ "image://taptimage/" + QString::fromUtf8(QUrl::toPercentEncoding(fi.absoluteFilePath())), fi.fileName() });
    }
    return results;
}
