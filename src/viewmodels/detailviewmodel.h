#ifndef DETAILVIEWMODEL_H
#define DETAILVIEWMODEL_H

#include <QObject>
#include <qqml.h>
#include <QString>
#include <QImage>
#include <algorithm>
#include "models/lutfilterslistmodel.h"

class DetailViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString imageUrl          READ imageUrl          NOTIFY imageUrlChanged)
    Q_PROPERTY(QString imageName         READ imageName         NOTIFY imageNameChanged)
    Q_PROPERTY(bool    hasImage          READ hasImage          NOTIFY hasImageChanged)

    Q_PROPERTY(qreal   hue               READ hue               NOTIFY hueChanged)
    Q_PROPERTY(qreal   brightness        READ brightness        NOTIFY brightnessChanged)
    Q_PROPERTY(qreal   saturation        READ saturation        NOTIFY saturationChanged)
    Q_PROPERTY(bool    flipped           READ flipped           NOTIFY flippedChanged)
    Q_PROPERTY(int     activeFilterIndex READ activeFilterIndex NOTIFY activeFilterIndexChanged)

    // Model filtrów/LUT — role: name (QString), previewUrl (QUrl), lutPath (QString)
    Q_PROPERTY(LutFiltersListModel* lutFiltersListModel READ lutFiltersListModel CONSTANT)

    // Limity suwaków barwy/jasności/nasycenia — jedno źródło prawdy zarówno
    // dla QML, jak i dla clampowania w applyChanges()/ loadForEditing().

    Q_PROPERTY(qreal brightnessMin READ brightnessMin CONSTANT)
    Q_PROPERTY(qreal brightnessMax READ brightnessMax CONSTANT)
    Q_PROPERTY(qreal saturationMin READ saturationMin CONSTANT)
    Q_PROPERTY(qreal saturationMax READ saturationMax CONSTANT)
    Q_PROPERTY(qreal hueMin        READ hueMin        CONSTANT)
    Q_PROPERTY(qreal hueMax        READ hueMax        CONSTANT)

public:
    explicit DetailViewModel(QObject *parent = nullptr);

    static DetailViewModel* create(QQmlEngine*, QJSEngine*)
    {
            return new DetailViewModel();
    }

    // Jasność: -50% … +50%, Nasycenie: -90% … +90%, Barwa: -180° … +180°
    // (znormalizowane do -1.0 … +1.0, tak jak oczekuje shader).
    static constexpr qreal kBrightnessMin = -0.5;
    static constexpr qreal kBrightnessMax = 0.5;
    static constexpr qreal kSaturationMin = -0.9;
    static constexpr qreal kSaturationMax = 0.9;
    static constexpr qreal kHueMin = -1.0;
    static constexpr qreal kHueMax = 1.0;

    QString imageUrl()          const { return m_imageUrl; }
    QString imageName()         const { return m_imageName; }
    bool    hasImage()          const { return !m_imageUrl.isEmpty() && m_originalImageValid; }
    qreal   hue()               const { return m_current.hue; }
    qreal   brightness()        const { return m_current.brightness; }
    qreal   saturation()        const { return m_current.saturation; }
    bool    flipped()           const { return m_current.flipped; }
    int     activeFilterIndex() const { return m_current.activeFilterIndex; }
    LutFiltersListModel* lutFiltersListModel() const { return m_lutFiltersListModel; }

    qreal   brightnessMin()     const { return kBrightnessMin; }
    qreal   brightnessMax()     const { return kBrightnessMax; }
    qreal   saturationMin()     const { return kSaturationMin; }
    qreal   saturationMax()     const { return kSaturationMax; }
    qreal   hueMin()            const { return kHueMin; }
    qreal   hueMax()            const { return kHueMax; }


    Q_INVOKABLE void setImage(const QString &url, const QString &name);

    Q_INVOKABLE void applyChanges(qreal hue, qreal brightness, qreal saturation, bool flipped, int filterIndex);
    Q_INVOKABLE void revertChanges();
    Q_INVOKABLE void setAsWallpaper();
    Q_INVOKABLE void addToPlaylist();

    Q_INVOKABLE void loadForEditing(
        const QString &playlistItemId,
        const QString &sourcePath,
        const QString &name,
        qreal hue, qreal brightness,
        qreal saturation, bool flipped,
        const QString &lutPath
    );

    // Ustawia stan obrazu, zapisuje m_editingPlaylistItemId i emituje imageLoaded().
    // Wywoływane z Main.qml zaraz po addItem(), aby automatyczne applyChanges()
    // od razu aktualizowało właściwy podgląd na osi czasu na żywo.

    Q_INVOKABLE void setEditingItemId(const QString &id);

    signals:
        void hasImageChanged();
        void imageUrlChanged();
        void imageNameChanged();

        void hueChanged();
        void brightnessChanged();
        void saturationChanged();
        void flippedChanged();
        void activeFilterIndexChanged();

        void stateReverted();
        void imageLoaded();

        void imageAdded(
            const QString &sourcePath,
        	const QString &name,
        	qreal hue,
        	qreal brightness,
        	qreal saturation,
        	bool flipped,
        	const QString &lutPath);

        // Emitowany z applyChanges()/revertChanges() przy edycji istniejącego elementu.
        // Łącz z TimelineViewModel::updateItem(), aby odświeżać PreviewImage w TimelinePanel na żywo.
        void itemEditApplied(
            const QString &id,
            qreal hue,
            qreal brightness,
            qreal saturation,
            bool flipped,
            const QString &lutPath);


private:

    static qreal clampBrightness(qreal v) { return std::clamp(v, kBrightnessMin, kBrightnessMax); }
    static qreal clampSaturation(qreal v) { return std::clamp(v, kSaturationMin, kSaturationMax); }
    static qreal clampHue(qreal v)        { return std::clamp(v, kHueMin, kHueMax); }

    LutFiltersListModel *m_lutFiltersListModel;

    struct ColorState
    {
        qreal hue         = 0.0;
        qreal brightness  = 0.0;
        qreal saturation  = 0.0;
        bool  flipped     = false;
        int   activeFilterIndex = -1;
    };


    QString m_originalImagePath;
    bool    m_originalImageValid = false;

    QString m_editingPlaylistItemId;
    ColorState m_current;

    QString m_imageUrl;
    QString m_imageName;

};


#endif // DETAILVIEWMODEL_H
