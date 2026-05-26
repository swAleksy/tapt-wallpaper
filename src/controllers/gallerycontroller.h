#ifndef GALLERYCONTROLLER_H
#define GALLERYCONTROLLER_H

#include <QObject>
#include <qqml.h>
#include "../services/galleryservice.h"
#include "../models/imagemodel.h"

class GalleryController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ImageModel* imageModel READ imageModel CONSTANT)
    QML_ELEMENT          
    QML_SINGLETON 
    
public:
    GalleryController(QObject *parent = nullptr);
    ~GalleryController();

    ImageModel* imageModel() const;

    // Metoda wywoływana z QML po kliknięciu przycisku
    Q_INVOKABLE void requestScan(const QUrl &folderUrl);
    Q_INVOKABLE void loadNextBatch();

signals:
    void scanFinished(int count);
    void allImagesLoaded();

private:
    GalleryService *m_service;
    ImageModel *m_model;
    QString m_currentFolder;
    int m_loadedCount = 0;
    bool m_isLoading = false;  
    static constexpr int BATCH_SIZE = 50;
};

#endif // GALLERYCONTROLLER_H
