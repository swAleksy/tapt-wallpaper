#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "models/playlistitem.h"
#include <QAbstractListModel>
#include <QObject>

class PlaylistModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        SourcePathRole,
        NameRole,
        HueRole,
        BrightnessRole,
        SaturationRole,
        FlippedRole,
        LutPathRole
    };
    // rowCount/data/roleNames analogicznie do ImagesModel

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addOrUpdate(const PlaylistItem& item); // po id
    void remove(const QString& id);
    void resetToDefaults(const QString& id); // edit = EditState::identity()

private:
    QList<PlaylistItem> m_items;

};

#endif // PLAYLISTMODEL_H
