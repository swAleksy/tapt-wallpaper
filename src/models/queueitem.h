#ifndef QUEUEITEM_H
#define QUEUEITEM_H

#include <QString>
#include "models/editstate.h"

// Indeksy dni tygodnia używane w QueueItem::weekdayMask (bit i = Weekday::X).
// Poniedziałek = 0, zgodnie z konwencją używaną w TimelinePanel (DayOfWeekTrack).
namespace Weekday {
enum Index {
    Monday = 0,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};
}

struct QueueItem {
    QString   id;            // QUuid::createUuid().toString()
    QString   sourcePath;    // ścieżka do ORYGINALNEGO, niezmienionego pliku
    QString   name;
    EditState edit;
    // Empty = not yet rendered. The daemon's resolvedPath() checks isEmpty()
    // and falls back to sourcePath. Do NOT use a magic string here — any
    // non-empty value is treated as a real path.
    QString   exportedPath;

    // Celowo NIEZALEŻNE od pozycji w kolejce (m_items), bo ta sama kolejka
    // jest współdzielona przez wszystkie tryby

    // Time of the day: zakres w minutach od północy, [scheduleStartMin, scheduleEndMin).
    // -1 = obraz nie ma przypisanego przedziału czasowego.
    int scheduleStartMin = -1;
    int scheduleEndMin = -1;

    // Day of week: maska bitowa dni tygodnia, bit 0 = Poniedziałek ... bit 6 = Niedziela.
    // 0 = obraz nieprzypisany do żadnego dnia,
    int weekdayMask = 0;
};

#endif // QUEUEITEM_H
