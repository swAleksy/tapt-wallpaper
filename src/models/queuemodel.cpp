#include "queuemodel.h"

QueueModel::QueueModel(QObject* parent)
    : QAbstractListModel(parent)
{
}


int QueueModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

QVariant QueueModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const QueueItem& item = m_items[index.row()];

    switch (role) {
    case IdRole: return item.id;
    case SourcePathRole: return item.sourcePath;
    case ExportedPathRole: return item.exportedPath;
    case NameRole: return item.name;
    case HueRole: return item.edit.hue;
    case BrightnessRole: return item.edit.brightness;
    case SaturationRole: return item.edit.saturation;
    case FlippedRole: return item.edit.flipped;
    case LutPathRole: return item.edit.lutPath;
    case ScheduleStartMinRole: return item.scheduleStartMin;
    case ScheduleEndMinRole: return item.scheduleEndMin;
    case WeekdayMaskRole: return item.weekdayMask;
    default: return {};
    }
}

QHash<int, QByteArray> QueueModel::roleNames() const
{
    return {
        { IdRole, "id" },
        { SourcePathRole, "sourcePath" },
        { ExportedPathRole, "exportedPath" },
        { NameRole, "name" },
        { HueRole, "hue" },
        { BrightnessRole, "brightness" },
        { SaturationRole, "saturation" },
        { FlippedRole, "flipped" },
        { LutPathRole, "lutPath" },
        { ScheduleStartMinRole, "scheduleStartMin" },
        { ScheduleEndMinRole, "scheduleEndMin" },
        { WeekdayMaskRole, "weekdayMask" }
    };
}

QString QueueModel::idAt(int row) const
{
    if (row < 0 || row >= m_items.size())
        return {};

    return m_items[row].id;
}

void QueueModel::addOrUpdate(const QueueItem& item)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == item.id) {
            m_items[i] = item;

            emit dataChanged(index(i), index(i),
                {HueRole, BrightnessRole, SaturationRole, FlippedRole, LutPathRole});
            return;
        }
    }

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();

    emit countChanged();
}

void QueueModel::remove(const QString& id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();

            emit countChanged();
            return;
        }
    }
}

void QueueModel::clear()
{
    if (m_items.isEmpty())
        return;

    beginResetModel();
    m_items.clear();
    endResetModel();

    emit countChanged();
}

void QueueModel::resetToDefaults(const QString& id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            m_items[i].edit = EditState::identity();

            emit dataChanged(index(i), index(i),
                {HueRole, BrightnessRole, SaturationRole, FlippedRole, LutPathRole});
            return;
        }
    }
}

void QueueModel::move(int from, int to)
{
    if (from == to || from < 0 || from >= m_items.size() || to < 0 || to >= m_items.size())
        return;

    // beginMoveRows wymaga "destination" jako indeksu PO przesunięciu w dół,
    // patrz dokumentacja QAbstractItemModel::beginMoveRows.
    const int destination = (to > from) ? to + 1 : to;
    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), destination))
        return;

    const QueueItem moved = m_items.takeAt(from);
    m_items.insert(to, moved);

    endMoveRows();
}

void QueueModel::setTimeSlot(const QString& id, int startMin, int endMin)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            m_items[i].scheduleStartMin = startMin;
            m_items[i].scheduleEndMin = endMin;

            emit dataChanged(index(i), index(i),
                {ScheduleStartMinRole, ScheduleEndMinRole});
            return;
        }
    }
}

void QueueModel::setWeekdayMask(const QString& id, int mask)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            m_items[i].weekdayMask = mask;

            emit dataChanged(index(i), index(i), {WeekdayMaskRole});
            return;
        }
    }
}

void QueueModel::assignDay(const QString& id, int day)
{
    if (day < 0 || day > 6)
        return;

    const int bit = 1 << day;
    for (int i = 0; i < m_items.size(); ++i) {
        const bool isTarget = (m_items[i].id == id);
        const bool hadBit = m_items[i].weekdayMask & bit;

        if (isTarget) {
            if (!hadBit) {
                m_items[i].weekdayMask |= bit;
                emit dataChanged(index(i), index(i), {WeekdayMaskRole});
            }
        } else if (hadBit) {
            // Zdejmij ten dzień każdemu innemu elementowi, który go miał —
            // dzień tygodnia może należeć maks. do jednego obrazu naraz.
            m_items[i].weekdayMask &= ~bit;
            emit dataChanged(index(i), index(i), {WeekdayMaskRole});
        }
    }
}

void QueueModel::unassignDay(const QString& id, int day)
{
    if (day < 0 || day > 6)
        return;

    const int bit = 1 << day;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            if (m_items[i].weekdayMask & bit) {
                m_items[i].weekdayMask &= ~bit;
                emit dataChanged(index(i), index(i), {WeekdayMaskRole});
            }
            return;
        }
    }
}
