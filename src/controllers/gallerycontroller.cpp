#include "gallerycontroller.h"
#include <qurl.h>

GalleryController::GalleryController(QObject *parent) : QObject(parent) {
    m_service = new GalleryService();
    m_model = new ImageModel();
}

GalleryController::~GalleryController() {
    delete m_service;
    delete m_model;
}

ImageModel* GalleryController::imageModel() const {
    return m_model;
}

void GalleryController::requestScan(const QUrl &folderUrl) {
    m_currentFolder = folderUrl.toLocalFile();
    m_loadedCount = 0;
    m_model->clear();          // ← potrzebujesz tej metody w ImageModel (patrz niżej)
    loadNextBatch();
}

void GalleryController::loadNextBatch() {
    if (m_isLoading || m_currentFolder.isEmpty()) return;
    m_isLoading = true;

    QList<ImageItem> items = m_service->scanFolder(m_currentFolder, m_loadedCount, BATCH_SIZE);

    if (items.isEmpty()) {
        emit allImagesLoaded();
        m_isLoading = false;
        return;
    }

    m_model->appendImages(items);  
    m_loadedCount += items.count();

    emit scanFinished(m_loadedCount);
    m_isLoading = false;
}