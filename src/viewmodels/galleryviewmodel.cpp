#include "galleryviewmodel.h"
#include <qobject.h>
#include <QUrl>

GalleryViewModel::GalleryViewModel(QObject *parent)
    : QObject(parent)
    , m_service(new GalleryService())
    , m_model(new ImagesModel(this))  // ← this jako parent: Qt automatycznie zwolni pamięć
    , m_detail(new DetailViewModel(this))
    {}


GalleryViewModel::~GalleryViewModel()
{
    delete m_service;
}


// Akcje wywoływane z QML

void GalleryViewModel::loadFolder(const QUrl &folderUrl) {
    const QString path = folderUrl.toLocalFile();
    if (path.isEmpty())
        return;

    m_currentFolder = path;
    m_loadedCount = 0;
    m_model->clear();
    m_detail->clear();  
    setErrorMessage("");
    setImageCount(0);
    setHasFolder(true);

    loadNextBatch();
}

void GalleryViewModel::loadNextBatch() {
    if (m_isLoading || m_currentFolder.isEmpty()) 
        return;

    setIsLoading(true);

    QList<ImageItem> items = m_service->scanFolder(m_currentFolder, m_loadedCount, BATCH_SIZE);

    if (items.isEmpty() && m_loadedCount == 0) {
        setErrorMessage("Brak zdjęć w tym folderze");
        setIsLoading(false);
        return;
    }

    if (!items.isEmpty()) {
        m_model->appendImages(items);
        m_loadedCount += items.count();
        setImageCount(m_loadedCount);
    }
 
    setIsLoading(false);
}

void GalleryViewModel::selectImage(int index)
{
    QModelIndex modelIndex = m_model->index(index, 0);
 
    if (!modelIndex.isValid())
        return;
 
    QString url  = m_model->data(modelIndex, ImagesModel::UrlRole).toString();
    QString name = m_model->data(modelIndex, ImagesModel::NameRole).toString();
 
    m_detail->setImage(url, name);

    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        emit selectedIndexChanged();
    }
}


// ── Prywatne settery ─────────────────────────────────────────────────────────

void GalleryViewModel::setIsLoading(bool value)
{
    if (m_isLoading == value) return;
    m_isLoading = value;
    emit isLoadingChanged();
}
 
void GalleryViewModel::setImageCount(int value)
{
    if (m_imageCount == value) return;
    m_imageCount = value;
    emit imageCountChanged();  // odświeża imageCount i isEmpty jednocześnie
}
 
void GalleryViewModel::setErrorMessage(const QString &value)
{
    if (m_errorMessage == value) return;
    m_errorMessage = value;
    emit errorMessageChanged();
}
 
void GalleryViewModel::setHasFolder(bool value)
{
    if (m_hasFolder == value) return;
    m_hasFolder = value;
    emit hasFolderChanged();
}
