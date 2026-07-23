#include "playlistmodel.h"

int PlaylistModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const PlaylistItem& item = m_items[index.row()];

    switch (role) {
    case IdRole: return item.id;
    case SourcePathRole: return item.sourcePath;
    case NameRole: return item.name;
    case HueRole: return item.edit.hue;
    case BrightnessRole: return item.edit.brightness;
    case SaturationRole: return item.edit.saturation;
    case FlippedRole: return item.edit.flipped;
    case LutPathRole: return item.edit.lutPath;
    default: return {};
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { SourcePathRole, "sourcePath" },
        { NameRole, "name" },
        { HueRole, "hue" },
        { BrightnessRole, "brightness" },
        { SaturationRole, "saturation" },
        { FlippedRole, "flipped" },
        { LutPathRole, "lutPath" }
    };
}

void PlaylistModel::addOrUpdate(const PlaylistItem& item)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == item.id) {
            m_items[i] = item;

            emit dataChanged(index(i), index(i));
            return;
        }
    }

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
}

void PlaylistModel::remove(const QString& id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

void PlaylistModel::resetToDefaults(const QString& id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            m_items[i].edit = EditState::identity();

            emit dataChanged(index(i), index(i));
            return;
        }
    }
}
