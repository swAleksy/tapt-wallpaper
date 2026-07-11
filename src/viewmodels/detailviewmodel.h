#ifndef DETAILVIEWMODEL_H
#define DETAILVIEWMODEL_H

#include <QObject>
#include <qqml.h>
#include <QString>
#include <QImage>
#include "lutfilterslistmodel.h"
#include "lutservice.h"

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
    Q_PROPERTY(bool    busy              READ busy              NOTIFY busyChanged) // NOWE: true, gdy LutService przetwarza obraz w tle

    // Model filtrów/LUT — role: name (QString), previewUrl (QUrl), lutPath (QString)
    Q_PROPERTY(LutFiltersListModel* lutFiltersListModel READ lutFiltersListModel CONSTANT)

public:
    explicit DetailViewModel(QObject *parent = nullptr);

    static DetailViewModel* create(QQmlEngine*, QJSEngine*)
    {
            return new DetailViewModel();
    }

    QString imageUrl()          const { return m_imageUrl; }
    QString imageName()         const { return m_imageName; }
    // bool    hasImage()          const { return !m_imageUrl.isEmpty() && !m_originalImage.isNull(); }
    bool    hasImage() const { return !m_imageUrl.isEmpty() && m_originalImageValid; }
    qreal   hue()               const { return m_current.hue; }
    qreal   brightness()        const { return m_current.brightness; }
    qreal   saturation()        const { return m_current.saturation; }
    bool    flipped()           const { return m_current.flipped; }
    int     activeFilterIndex() const { return m_current.activeFilterIndex; }
    bool    busy()               const { return m_busy; }
    LutFiltersListModel* lutFiltersListModel() const { return m_lutFiltersListModel; }

    // void clear();

    Q_INVOKABLE void setImage(const QString &url, const QString &name);

    Q_INVOKABLE void applyChanges(qreal hue, qreal brightness, qreal saturation, bool flipped, int filterIndex);
    Q_INVOKABLE void revertChanges();   // reset do stanu sprzed ostatniego Apply
    Q_INVOKABLE void setAsWallpaper();
    Q_INVOKABLE void addToPlaylist();

signals:
    void hasImageChanged();
    void imageUrlChanged();
    void imageNameChanged();

    void hueChanged();
    void brightnessChanged();
    void saturationChanged();
    void flippedChanged();
    void activeFilterIndexChanged();
    void busyChanged();

    void stateReverted();
    void imageLoaded();

private:
    void reprocessAsync(); // wspólna logika applyChanges()/revertChanges(): wybiera ścieżkę LUT-a,
                            // ustawia busy i odpala LutService::applyChangesAsync

    LutFiltersListModel *m_lutFiltersListModel;
    LutService *m_lutService;

    struct ColorState
    {
        qreal hue         = 0.0;
        qreal brightness  = 0.0;
        qreal saturation  = 0.0;
        bool  flipped     = false;
        int   activeFilterIndex = -1;
    };

    // QImage m_originalImage;    // wczytany raz, nigdy nie ruszany

    QString m_originalImagePath;   // zamiast QImage m_originalImage
    bool    m_originalImageValid = false;

    // QImage m_lutProcessed;     // wynik LUT, cache — nie przeliczaj przy każdym HSB sliderze
    bool   m_busy = false;     // USUNIĘTO: nieużywane QFutureWatcher<QImage> m_lutWatcher (LutService ma własny)

    ColorState m_current;
    // ColorState m_committed;

    QString m_imageUrl;
    QString m_imageName;

};


#endif // DETAILVIEWMODEL_H
