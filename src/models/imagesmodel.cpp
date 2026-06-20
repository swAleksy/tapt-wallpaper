#include "imagesmodel.h"

int ImagesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_images.count();
}

// Ta funkcja zwraca konkretne pole dla danego wiersza (wywoływana przez QML)
QVariant ImagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_images.count())
        return QVariant();

    const ImageItem &item = m_images[index.row()];
    if (role == UrlRole)
        return item.url;
    if (role == NameRole)
        return item.name;

    return QVariant();
}

// Definiujemy nazwy zmiennych, których użyjemy w QML (tzw. Roles)
QHash<int, QByteArray> ImagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[UrlRole] = "imageUrl";
    roles[NameRole] = "imageName";
    return roles;
}

void ImagesModel::appendImages(const QList<ImageItem> &images)
{
    if (images.isEmpty())
        return;

    int first = m_images.count();
    int last = first + images.count() - 1;
    beginInsertRows(QModelIndex(), first, last);  // ← NIE resetuje widoku
    m_images.append(images);
    endInsertRows();
}

void ImagesModel::clear()
{
    beginResetModel();
    m_images.clear();
    endResetModel();
}

ImageItem ImagesModel::imageAt(int row) const
{
    if (row < 0 || row >= m_images.size())
        return {};

    return m_images.at(row);
}
