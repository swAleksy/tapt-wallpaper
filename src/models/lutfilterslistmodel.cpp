#include "lutfilterslistmodel.h"
#include <qdiriterator.h>

LutFiltersListModel::LutFiltersListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

// Liczba elementów na liście (potrzebne dla QML)
int LutFiltersListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.count();
}

QVariant LutFiltersListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.count())
        return QVariant();

    const FilterItem &item = m_items[index.row()];

    switch (role) {
    case NameRole:
        return item.name;
    case PreviewUrlRole:
        return item.previewUrl;
    case LutPathRole:
        return item.lutPath;
    }

    return QVariant();
}


// Mapowanie ról na stringi, których użyjesz w QML (np. model.name, model.previewUrl)
QHash<int, QByteArray> LutFiltersListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PreviewUrlRole] = "previewUrl";
    roles[LutPathRole] = "lutPath";
    return roles;
}

// Metody Q_INVOKABLE, o które krzyczy linker
QString LutFiltersListModel::filterName(int index) const
{
    if (index < 0 || index >= m_items.count())
        return QString();
    return m_items[index].name;
}

QString LutFiltersListModel::lutPath(int index) const
{
    if (index < 0 || index >= m_items.count())
        return QString();
    return m_items[index].lutPath;
}

void LutFiltersListModel::loadFromDirectory(const QString &dir)
{
    QDirIterator it(dir, {"*.cube"}, QDir::Files);
    QList<FilterItem> items;
    while (it.hasNext()) {
        it.next();
        QFileInfo fi = it.fileInfo();
        FilterItem item;
        item.name    = fi.baseName();
        //item.lutPath = fi.absoluteFilePath();
        item.lutPath = it.filePath();
        // previewUrl: szukaj pliku .png o tej samej nazwie obok .cube
        // QString pngPath = fi.absolutePath() + "/" + fi.baseName() + ".png";
        // item.previewUrl = QFile::exists(pngPath)
        //                   ? QUrl::fromLocalFile(pngPath)
        //                   : QUrl{};
        QString pngPath = ":/luts/" + fi.baseName() + ".png";  // bezpieczna, jawna ścieżka
        item.previewUrl = QFile::exists(pngPath)
                          ? QUrl(QStringLiteral("qrc") + pngPath)  // "qrc:/luts/plik.png"
                          : QUrl{};
        items.append(item);
    }
    beginResetModel();
    m_items = items;
    endResetModel();
}
