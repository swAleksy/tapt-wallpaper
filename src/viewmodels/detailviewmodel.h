#ifndef DETAILVIEWMODEL_H
#define DETAILVIEWMODEL_H

#include <QObject>
#include <QString>


class DetailViewModel : public QObject 
{
    Q_OBJECT
 
    Q_PROPERTY(QString imageUrl  READ imageUrl  NOTIFY imageChanged)
    Q_PROPERTY(QString imageName READ imageName NOTIFY imageChanged)
    Q_PROPERTY(bool    hasImage  READ hasImage  NOTIFY imageChanged)


public:
    explicit DetailViewModel(QObject *parent = nullptr);
 
    QString imageUrl()  const { return m_imageUrl; }
    QString imageName() const { return m_imageName; }
    bool    hasImage()  const { return !m_imageUrl.isEmpty(); }
 
    // Wywoływane przez GalleryViewModel, nie przez QML bezpośrednio
    void setImage(const QString &url, const QString &name);
    void clear();
 
signals:
    void imageChanged();  // jeden sygnał dla wszystkich trzech property
 
private:
    QString m_imageUrl;
    QString m_imageName;
};


#endif // DETAILVIEWMODEL_H