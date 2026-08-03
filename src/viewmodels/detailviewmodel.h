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

    // Limity suwaków jasności/nasycenia — jedno źródło prawdy zarówno dla
    // QML (Slider.from/to), jak i dla clampowania w applyChanges()/
    // loadForEditing(), żeby granice nigdy nie rozjechały się między UI
    // a walidacją w C++.
    Q_PROPERTY(qreal brightnessMin READ brightnessMin CONSTANT)
    Q_PROPERTY(qreal brightnessMax READ brightnessMax CONSTANT)
    Q_PROPERTY(qreal saturationMin READ saturationMin CONSTANT)
    Q_PROPERTY(qreal saturationMax READ saturationMax CONSTANT)

public:
    explicit DetailViewModel(QObject *parent = nullptr);

    static DetailViewModel* create(QQmlEngine*, QJSEngine*)
    {
            return new DetailViewModel();
    }

    // Jasność: -50% … +50%, Nasycenie: -90% … +90%.
    static constexpr qreal kBrightnessMin = -0.5;
    static constexpr qreal kBrightnessMax = 0.5;
    static constexpr qreal kSaturationMin = -0.9;
    static constexpr qreal kSaturationMax = 0.9;

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
    // ustawia m_originalImagePath = sourcePath, m_current = {hue, brightness, ...},
    // zapamiętuje m_editingPlaylistItemId = playlistItemId, emituje imageLoaded()

    // Wywoływane z warstwy wyżej (Main.qml) zaraz po tym, jak
    // TimelineViewModel::addItem() zwróci id dla elementu utworzonego przez
    // addToPlaylist(). Dzięki temu kolejne automatyczne applyChanges()
    // (debounce w DetailView.qml) trafiają już do właściwego elementu
    // kolejki i podgląd na osi czasu aktualizuje się na żywo — bez
    // konieczności ponownego wejścia w "Edytuj" z timeline'u.
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

        // Emitowany z applyChanges() i revertChanges(), gdy edytujemy element
        // JUŻ obecny w kolejce (czyli po loadForEditing() lub po
        // addToPlaylist() + setEditingItemId()). Łącz w warstwie wyżej z
        // TimelineViewModel::updateItem(id, ...), żeby PreviewImage w
        // TimelinePanel dostał nowe wartości na żywo.
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
