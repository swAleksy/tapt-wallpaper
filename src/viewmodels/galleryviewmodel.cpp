#include "galleryviewmodel.h"
#include <qobject.h>
#include <QUrl>
#include <QtConcurrent>

GalleryViewModel::GalleryViewModel(QObject *parent)
    : QObject(parent)
    , m_service(std::make_shared<GalleryService>())
    , m_model(new ImagesModel(this))
    , m_detail(new DetailViewModel(this))
    {
        connect(&m_scanWatcher, &QFutureWatcher<QList<ImageItem>>::finished, this, [this]() {
            m_allItems = m_scanWatcher.result();
            m_loadedCount = 0;
            setIsLoading(false);
            loadNextBatch();
        });
    }

GalleryViewModel::~GalleryViewModel()
{}

void GalleryViewModel::loadFolder(const QUrl &folderUrl)
{
    const QString path = folderUrl.toLocalFile();
    if (path.isEmpty() || m_isLoading)
        return;

    m_currentFolder = path;
    m_model->clear();
    m_detail->clear();
    m_allItems.clear();

    resetSelectedIndex();
    setErrorMessage("");
    setImageCount(0);
    setHasFolder(true);
    setIsLoading(true);

    std::shared_ptr<GalleryService> service = m_service;
    m_scanWatcher.setFuture(QtConcurrent::run([service, path]() {
        return service->scanFolder(path);
    }));
}

void GalleryViewModel::loadNextBatch()
{
    if (m_isLoading || m_allItems.isEmpty())
        return;
    if (m_loadedCount >= m_allItems.count())
        return;

    QList<ImageItem> batch = m_allItems.mid(m_loadedCount, BATCH_SIZE);
    m_model->appendImages(batch);
    m_loadedCount += batch.count();
    setImageCount(m_loadedCount);

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

void GalleryViewModel::setIsLoading(bool value)
{
    if (m_isLoading == value)
        return;
    m_isLoading = value;
    emit isLoadingChanged();
    emit isEmptyChanged();
}

void GalleryViewModel::setImageCount(int value)
{
    if (m_imageCount == value)
        return;
    m_imageCount = value;
    emit imageCountChanged();
    emit isEmptyChanged();
}

void GalleryViewModel::setErrorMessage(const QString &value)
{
    if (m_errorMessage == value)
        return;
    m_errorMessage = value;
    emit errorMessageChanged();
}

void GalleryViewModel::setHasFolder(bool value)
{
    if (m_hasFolder == value)
        return;
    m_hasFolder = value;
    emit hasFolderChanged();
    emit isEmptyChanged();
}

void GalleryViewModel::resetSelectedIndex()
{
    if (m_selectedIndex == -1)
        return;
    m_selectedIndex = -1;
    emit selectedIndexChanged();
}
