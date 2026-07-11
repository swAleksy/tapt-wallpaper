#ifndef IMAGESMODEL_H
#define IMAGESMODEL_H

#include "imageItem.h"
#include <QAbstractListModel>

class ImagesModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ImagesModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    enum ImageRoles { UrlRole = Qt::UserRole + 1, NameRole };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void appendImages(const QList<ImageItem>& images);
    void clear();
    ImageItem imageAt(int row) const;

private:
    QList<ImageItem> m_images;
};

#endif
