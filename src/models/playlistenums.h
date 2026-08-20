#ifndef PLAYLISTENUMS_H
#define PLAYLISTENUMS_H
#include <QObject>
#include <QString>
#include <qqml.h>

// Enumy stanu playlisty, współdzielone przez TimelineViewModel i
// MonitorPlaylistState — zamiast surowych intów (0/1/2/3)
//
// QML_ELEMENT: namespace jest zarejestrowany jako typ QML, więc w .qml da
// się pisać PlaylistEnums.Mode.DayOfWeek
namespace PlaylistEnums {
Q_NAMESPACE
QML_ELEMENT

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

}

#endif // PLAYLISTENUMS_H
