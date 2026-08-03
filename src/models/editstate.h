#ifndef EDITSTATE_H
#define EDITSTATE_H

#include <QString>
#include <QtTypes>

struct EditState {
    qreal hue = 0.0;
    qreal brightness = 0.0;
    qreal saturation = 0.0;
    bool flipped = false;
    QString lutPath; // np. ":/luts/vintage.cube" — NIE indeks,
                     // bo indeks przestaje być stabilny poza
                     // sesją UI

    static EditState identity()
    {
        return EditState { };
    }

    bool operator==(const EditState& o) const
    {
        return hue == o.hue
            && brightness == o.brightness
            && saturation == o.saturation
            && flipped == o.flipped
            && lutPath == o.lutPath;
    }
};

#endif // EDITSTATE_H
