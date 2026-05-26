#include "imagemodel.h"

int ImageModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_images.count();
}

// Ta funkcja zwraca konkretne pole dla danego wiersza (wywoływana przez QML)
QVariant ImageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_images.count()) return QVariant();

    const ImageItem &item = m_images[index.row()];
    if (role == UrlRole) return item.url;
    if (role == NameRole) return item.name;

    return QVariant();
}

// Definiujemy nazwy zmiennych, których użyjemy w QML (tzw. Roles)
QHash<int, QByteArray> ImageModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[UrlRole] = "imageUrl";
    roles[NameRole] = "imageName";
    return roles;
}

// void ImageModel::setImages(const QList<ImageItem> &images) {
//     beginResetModel(); // Informujemy QML: "Zaraz zmieni się cała lista, przygotuj się"
//     m_images = images;
//     endResetModel();   // Informujemy QML: "Koniec zmian, odśwież widok"
// }

void ImageModel::appendImages(const QList<ImageItem> &images) {
    if (images.isEmpty()) 
        return;
    
    int first = m_images.count();
    int last = first + images.count() - 1;
    beginInsertRows(QModelIndex(), first, last);  // ← NIE resetuje widoku
    m_images.append(images);
    endInsertRows();
}

void ImageModel::clear() {
    beginResetModel();
    m_images.clear();
    endResetModel();
}