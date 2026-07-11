#include "lutfilterslistmodel.h"
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

LutFiltersListModel::LutFiltersListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

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
    case LutPathRole:
        return item.lutPath;
    case SizeRole: // <-- Dodana obsługa roli w data()
        return item.size;
    }

    return QVariant();
}

QHash<int, QByteArray> LutFiltersListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[LutPathRole] = "lutPath";
    roles[SizeRole] = "size"; // <-- Rejestracja roli dla QML (użycie jako model.size)
    return roles;
}

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

int LutFiltersListModel::filterSize(int index) const
{
    if (index < 0 || index >= m_items.count())
        return 0;
    return m_items[index].size;
}

void LutFiltersListModel::loadFromDirectory(const QString &dir)
{
    QDirIterator it(dir, {"*.cube"}, QDir::Files);
    QList<FilterItem> items;

    while (it.hasNext()) {
        it.next();
        QFileInfo fi = it.fileInfo();
        FilterItem item;
        item.name = fi.baseName();
        item.lutPath = it.filePath();

        // --- Parsowanie pliku .cube w poszukiwaniu rozmiaru ---
        QFile file(it.filePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();

                // Szukamy linijki zaczynającej się od rozmiaru tablicy 3D lub 1D
                if (line.startsWith("LUT_3D_SIZE") || line.startsWith("LUT_1D_SIZE")) {
                    // simplified() zamienia tabulatory i wielokrotne spacje na pojedynczą spację
                    QStringList tokens = line.simplified().split(' ');
                    if (tokens.size() >= 2) {
                        bool ok;
                        int parsedSize = tokens[1].toInt(&ok);
                        if (ok) {
                            item.size = parsedSize;
                        }
                    }
                    break; // Rozmiar znaleziony, nie ma sensu czytać reszty (często ogromnego) pliku
                }

                // Optymalizacja: Jeśli natrafimy na pierwszą liczbę (dane LUT),
                // a nie znaleźliśmy nagłówka, to znaczy, że metadanych już nie ma.
                if (!line.isEmpty() && line[0].isDigit()) {
                    break;
                }
            }
            file.close();
        }
        // -----------------------------------------------------

        items.append(item);
    }

    beginResetModel();
    m_items = items;
    endResetModel();
}
