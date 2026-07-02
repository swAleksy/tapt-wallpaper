#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QUrl>
#include <qqml.h>

class LutFiltersListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        NameRole       = Qt::UserRole + 1,
        PreviewUrlRole,   // miniaturka z folderu z LUT-ami
        LutPathRole       // pełna ścieżka do .cube
    };
    Q_ENUM(Roles)

    struct FilterItem {
        QString name;
        QUrl    previewUrl;
        QString lutPath;
    };

    explicit LutFiltersListModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex & = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadFromDirectory(const QString &dir); // skanuj folder z .cube + .png

    // Wygodne dla QML — zamiast m.data(m.index(...), role)
    Q_INVOKABLE QString filterName(int index) const;
    Q_INVOKABLE QString lutPath(int index) const;

private:
    QList<FilterItem> m_items;
};
