#ifndef GALLERYVIEWMODEL_H
#define GALLERYVIEWMODEL_H

#include <QObject>
#include <QStandardPaths>
#include <QUrl>
#include <qstandardpaths.h>
#include <qtmetamacros.h>
#include <memory>
#include <QFutureWatcher>
#include <qqml.h>

#include "services/galleryservice.h"
#include "models/imagesmodel.h"

class GalleryViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(ImagesModel* imagesModel READ imagesModel CONSTANT)
    Q_PROPERTY(bool        isLoading    READ isLoading    NOTIFY isLoadingChanged)
    Q_PROPERTY(int         imageCount   READ imageCount   NOTIFY imageCountChanged)
    Q_PROPERTY(QString     errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool        hasFolder    READ hasFolder    NOTIFY hasFolderChanged)
    Q_PROPERTY(bool        isEmpty      READ isEmpty      NOTIFY isEmptyChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QUrl defaultDir READ defaultDir CONSTANT) // default picture dir location

public:
    explicit GalleryViewModel(QObject *parent = nullptr);
    ~GalleryViewModel();

    static GalleryViewModel* create(QQmlEngine*, QJSEngine*)
    {
        return new GalleryViewModel();
    }

    // Gettery — czyta Q_PROPERTY
    ImagesModel* imagesModel()  const   { return m_model; }
    bool isLoading()            const   { return m_isLoading; }
    int imageCount()            const   { return m_imageCount; }
    QString errorMessage()      const   { return m_errorMessage; }
    bool hasFolder()            const   { return m_hasFolder; }
    bool isEmpty()              const   { return m_hasFolder && m_imageCount == 0 && !m_isLoading; }
    QUrl defaultDir()           const   { return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)); }
    int selectedIndex()         const   { return m_selectedIndex; }

    // Akcje wywoływane z QML
    Q_INVOKABLE void loadFolder(const QUrl &folderUrl);
    Q_INVOKABLE void loadNextBatch();
    Q_INVOKABLE void selectImage(int index);

signals:
    void isLoadingChanged();
    void imageCountChanged();
    void errorMessageChanged();
    void hasFolderChanged();
    void isEmptyChanged();
    void selectedIndexChanged();
    void imageSelected(const QString &url, const QString &name);

private:
    // Prywatne settery — emitują sygnały, QML samo się odświeża
    void setIsLoading(bool value);
    void setImageCount(int value);
    void setErrorMessage(const QString &value);
    void setHasFolder(bool value);
    void resetSelectedIndex();

    std::shared_ptr<GalleryService> m_service;
    ImagesModel     *m_model;

    QString         m_currentFolder;
    int             m_loadedCount   = 0;
    bool            m_isLoading     = false;
    int             m_imageCount    = 0;
    bool            m_hasFolder     = false;
    QString         m_errorMessage;
    int             m_selectedIndex = -1;
    QList<ImageItem> m_allItems;

    QFutureWatcher<QList<ImageItem>> m_scanWatcher;

    static constexpr int BATCH_SIZE = 50;
};

#endif // GALLERYVIEWMODEL_H
