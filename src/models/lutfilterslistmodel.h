#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QUrl>
#include <qqml.h>
#include <qtmetamacros.h>

class LutFiltersListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        LutPathRole,       // pełna ścieżka do .cube
        SizeRole
    };
    Q_ENUM(Roles)

    struct FilterItem {
        QString name;
        QString lutPath;
        int size;
    };

    explicit LutFiltersListModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex & = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadFromDirectory(const QString &dir); // skanuj folder z .cube + .png

    // Wygodne dla QML — zamiast m.data(m.index(...), role)
    Q_INVOKABLE QString filterName(int index) const;
    Q_INVOKABLE QString lutPath(int index) const;
    Q_INVOKABLE int filterSize(int index) const;

private:
    QList<FilterItem> m_items;
};
