#include "models/queuemodel.h"
#include <algorithm>

namespace {
constexpr int kMinSlotMinutes = 30;
constexpr int kSnapMinutes = 5;
constexpr int kMaxWeekdayItems = 7;

int snapTo(qreal val, int step) { return qRound(val / step) * step; }

// Decodes the contiguous [startDay, endDay) run of set bits in mask.
// Only distributeWeekdaysEvenly()/moveWeekdayDivider() ever write
// weekdayMask for this feature, so contiguity holds — but this doesn't
// verify it. If something else starts calling setWeekdayMask()/assignDay()
// on these same items, that assumption breaks silently.
std::pair<int, int> decodeDayRange(int mask)
{
    if (mask == 0) return {0, 0};
    int lo = 0;
    while (!(mask & (1 << lo))) ++lo;
    int hi = 6;
    while (!(mask & (1 << hi))) --hi;
    return {lo, hi + 1};
}

int maskForRange(int startDay, int endDay)
{
    int mask = 0;
    for (int d = startDay; d < endDay; ++d) mask |= (1 << d);
    return mask;
}
} // namespace

QueueModel::QueueModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int QueueModel::findRow(const QString& id) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id)
            return i;
    }
    return -1;
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
    const int row = findRow(item.id);
    if (row != -1) {
        m_items[row] = item;

        emit dataChanged(index(row), index(row), {
            SourcePathRole, ExportedPathRole, NameRole,
            HueRole, BrightnessRole, SaturationRole, FlippedRole, LutPathRole,
            ScheduleStartMinRole, ScheduleEndMinRole, WeekdayMaskRole
        });
        return;
    }

    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();

    emit countChanged();
}

void QueueModel::remove(const QString& id)
{
    const int row = findRow(id);
    if (row == -1)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();

    emit countChanged();
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
    const int row = findRow(id);
    if (row == -1)
        return;

    m_items[row].edit = EditState::identity();

    emit dataChanged(index(row), index(row),
        {HueRole, BrightnessRole, SaturationRole, FlippedRole, LutPathRole});
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
    const int row = findRow(id);
    if (row == -1)
        return;

    m_items[row].scheduleStartMin = startMin;
    m_items[row].scheduleEndMin = endMin;

    emit dataChanged(index(row), index(row),
        {ScheduleStartMinRole, ScheduleEndMinRole});
}

void QueueModel::setWeekdayMask(const QString& id, int mask)
{
    const int row = findRow(id);
    if (row == -1)
        return;

    m_items[row].weekdayMask = mask;

    emit dataChanged(index(row), index(row), {WeekdayMaskRole});
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

    const int row = findRow(id);
    if (row == -1)
        return;

    const int bit = 1 << day;
    if (m_items[row].weekdayMask & bit) {
        m_items[row].weekdayMask &= ~bit;
        emit dataChanged(index(row), index(row), {WeekdayMaskRole});
    }
}


void QueueModel::distributeTimeSlotsEvenly()
{
    const int count = m_items.size();
    for (int i = 0; i < count; ++i) {
        const int start = qRound(i * 1440.0 / count);
        const int end = (i == count - 1) ? 1440 : qRound((i + 1) * 1440.0 / count);
        if (m_items[i].scheduleStartMin != start || m_items[i].scheduleEndMin != end) {
            m_items[i].scheduleStartMin = start;
            m_items[i].scheduleEndMin = end;
            emit dataChanged(index(i), index(i), {ScheduleStartMinRole, ScheduleEndMinRole});
        }
    }
}

void QueueModel::moveTimeSlotDivider(int dividerIndex, qreal proposedBoundaryMin)
{
    if (dividerIndex < 0 || dividerIndex + 1 >= m_items.size())
        return;

    QueueItem& left = m_items[dividerIndex];
    QueueItem& right = m_items[dividerIndex + 1];

    int clamped = snapTo(proposedBoundaryMin, kSnapMinutes);
    clamped = std::max(left.scheduleStartMin + kMinSlotMinutes,
                        std::min(clamped, right.scheduleEndMin - kMinSlotMinutes));

    left.scheduleEndMin = clamped;
    right.scheduleStartMin = clamped;

    emit dataChanged(index(dividerIndex), index(dividerIndex), {ScheduleEndMinRole});
    emit dataChanged(index(dividerIndex + 1), index(dividerIndex + 1), {ScheduleStartMinRole});
}

int QueueModel::scheduleStartMinAt(int row) const
{
    return (row >= 0 && row < m_items.size()) ? m_items[row].scheduleStartMin : 0;
}

int QueueModel::scheduleEndMinAt(int row) const
{
    return (row >= 0 && row < m_items.size()) ? m_items[row].scheduleEndMin : 0;
}

void QueueModel::distributeWeekdaysEvenly()
{
    const int count = std::min<int>(m_items.size(), kMaxWeekdayItems);
    for (int i = 0; i < m_items.size(); ++i) {
        int mask = 0;
        if (i < count) {
            const int start = qRound(i * 7.0 / count);
            const int end = (i == count - 1) ? 7 : qRound((i + 1) * 7.0 / count);
            mask = maskForRange(start, end);
        }
        if (m_items[i].weekdayMask != mask) {
            m_items[i].weekdayMask = mask;
            emit dataChanged(index(i), index(i), {WeekdayMaskRole});
        }
    }
}

void QueueModel::moveWeekdayDivider(int dividerIndex, qreal proposedBoundaryDay)
{
    const int scheduledCount = std::min<int>(m_items.size(), kMaxWeekdayItems);
    if (dividerIndex < 0 || dividerIndex + 1 >= scheduledCount)
        return;

    auto [leftStart, leftEnd] = decodeDayRange(m_items[dividerIndex].weekdayMask);
    auto [rightStart, rightEnd] = decodeDayRange(m_items[dividerIndex + 1].weekdayMask);

    int clamped = qRound(proposedBoundaryDay);
    clamped = std::max(leftStart + 1, std::min(clamped, rightEnd - 1));

    m_items[dividerIndex].weekdayMask = maskForRange(leftStart, clamped);
    m_items[dividerIndex + 1].weekdayMask = maskForRange(clamped, rightEnd);

    emit dataChanged(index(dividerIndex), index(dividerIndex), {WeekdayMaskRole});
    emit dataChanged(index(dividerIndex + 1), index(dividerIndex + 1), {WeekdayMaskRole});
}

int QueueModel::scheduleStartDayAt(int row) const
{
    return (row >= 0 && row < m_items.size()) ? decodeDayRange(m_items[row].weekdayMask).first : 0;
}

int QueueModel::scheduleEndDayAt(int row) const
{
    return (row >= 0 && row < m_items.size()) ? decodeDayRange(m_items[row].weekdayMask).second : 0;
}
