#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QAbstractListModel>
#include "imageItem.h"

class ImageModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum ImageRoles {
        UrlRole = Qt::UserRole + 1,
        NameRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Metoda do aktualizacji danych z serwisu
    // void setImages(const QList<ImageItem> &images);
    void appendImages(const QList<ImageItem> &images); // ← nowa, dodaje na końcu
    void clear();      

private:
    QList<ImageItem> m_images;
};

#endif