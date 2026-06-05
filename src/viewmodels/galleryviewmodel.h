#ifndef GALLERYVIEWMODEL_H
#define GALLERYVIEWMODEL_H

#include <QObject>
#include <qtmetamacros.h>
#include "../services/galleryservice.h"
#include "../models/imagesmodel.h"
#include "detailviewmodel.h"

class GalleryViewModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ImagesModel* imagesModel READ imagesModel    CONSTANT)
    Q_PROPERTY(DetailViewModel* detail  READ detail         CONSTANT)
    Q_PROPERTY(bool        isLoading    READ isLoading    NOTIFY isLoadingChanged)
    Q_PROPERTY(int         imageCount   READ imageCount   NOTIFY imageCountChanged)
    Q_PROPERTY(QString     errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool        hasFolder    READ hasFolder    NOTIFY hasFolderChanged)
    Q_PROPERTY(bool        isEmpty      READ isEmpty      NOTIFY imageCountChanged)


public:
    explicit GalleryViewModel(QObject *parent = nullptr);
    ~GalleryViewModel();

    // Gettery — czyta Q_PROPERTY
    ImagesModel* imagesModel() const  { return m_model; }
    DetailViewModel* detail() const { return m_detail; }
    bool   isLoading()    const     { return m_isLoading; }
    int    imageCount()   const     { return m_imageCount; }
    QString errorMessage() const    { return m_errorMessage; }
    bool   hasFolder()    const     { return m_hasFolder; }
    bool   isEmpty()      const     { return m_hasFolder && m_imageCount == 0 && !m_isLoading; }

    // Akcje wywoływane z QML
    Q_INVOKABLE void loadFolder(const QUrl &folderUrl);
    Q_INVOKABLE void loadNextBatch();
    Q_INVOKABLE void selectImage(int index);

signals:
    void isLoadingChanged();
    void imageCountChanged();
    void errorMessageChanged();
    void hasFolderChanged();

private:
    // Prywatne settery — emitują sygnały, QML samo się odświeża
    void setIsLoading(bool value);
    void setImageCount(int value);
    void setErrorMessage(const QString &value);
    void setHasFolder(bool value);

    GalleryService  *m_service;
    ImagesModel     *m_model;
    DetailViewModel *m_detail;

    QString         m_currentFolder;
    int             m_loadedCount   = 0;
    bool            m_isLoading     = false;
    int             m_imageCount    = 0;
    bool            m_hasFolder     = false;
    QString         m_errorMessage;
 
    static constexpr int BATCH_SIZE = 50;
};

#endif // GALLERYVIEWMODEL_H