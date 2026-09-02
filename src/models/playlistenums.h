#ifndef PLAYLISTENUMS_H
#define PLAYLISTENUMS_H
#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

// Enumy stanu playlisty, współdzielone przez TimelineViewModel i
// MonitorPlaylistState — zamiast surowych intów (0/1/2/3)
//
// QML_ELEMENT: namespace jest zarejestrowany jako typ QML, więc w .qml da
// się pisać PlaylistEnums.Mode.DayOfWeek
namespace PlaylistEnums {
Q_NAMESPACE
QML_ELEMENT

// Jedyne źródło prawdy dla "7 dni tygodnia" w trybie Day of week.
// Wcześniej ta sama wartość była zaszyta osobno jako literał w trzech
// miejscach (TimelineViewModel::kMaxDayOfWeekItems, QueueModel's
// kMaxWeekdayItems, DayOfWeekTrack.qml's dayCount) bez wspólnego źródła —
// zmiana jednej nie gwarantowała zmiany pozostałych.
constexpr int kDaysInWeek = 7;

enum class Mode {
    TimeOfDay = 0,
    WhenLoggingIn = 1,
    OnATimer = 2,
    DayOfWeek = 3
};
Q_ENUM_NS(Mode)

// Wspólne dla "When logging in" i "On a timer"
enum class OrderMode {
    Random = 0,
    Ordered = 1
};
Q_ENUM_NS(OrderMode)

// Jednostka interwału w trybie "On a timer"
enum class IntervalUnit {
    Minutes = 0,
    Hours = 1
};
Q_ENUM_NS(IntervalUnit)

inline QString toString(Mode mode)
{
    switch (mode)
    {
        case Mode::TimeOfDay:     return QStringLiteral("timeOfDay");
        case Mode::WhenLoggingIn: return QStringLiteral("whenLoggingIn");
        case Mode::OnATimer:      return QStringLiteral("onATimer");
        case Mode::DayOfWeek:     return QStringLiteral("dayOfWeek");
    }
    return QStringLiteral("timeOfDay");
}

inline QString toString(OrderMode mode)
{
    switch (mode)
    {
        case OrderMode::Random:  return QStringLiteral("random");
        case OrderMode::Ordered: return QStringLiteral("ordered");
    }
    return QStringLiteral("random");
}

inline QString toString(IntervalUnit unit)
{
    switch (unit)
    {
        case IntervalUnit::Minutes: return QStringLiteral("minutes");
        case IntervalUnit::Hours:   return QStringLiteral("hours");
    }
    return QStringLiteral("minutes");
}

inline bool fromString(const QString& s, Mode& out)
{
    if (s == QStringLiteral("timeOfDay"))     { out = Mode::TimeOfDay; return true; }
    if (s == QStringLiteral("whenLoggingIn")) { out = Mode::WhenLoggingIn; return true; }
    if (s == QStringLiteral("onATimer"))      { out = Mode::OnATimer; return true; }
    if (s == QStringLiteral("dayOfWeek"))     { out = Mode::DayOfWeek; return true; }
    return false;
}

inline bool fromString(const QString& s, OrderMode& out)
{
    if (s == QStringLiteral("random"))  { out = OrderMode::Random; return true; }
    if (s == QStringLiteral("ordered")) { out = OrderMode::Ordered; return true; }
    return false;
}

inline bool fromString(const QString& s, IntervalUnit& out)
{
    if (s == QStringLiteral("minutes")) { out = IntervalUnit::Minutes; return true; }
    if (s == QStringLiteral("hours"))   { out = IntervalUnit::Hours; return true; }
    return false;
}

}

#endif // PLAYLISTENUMS_H
