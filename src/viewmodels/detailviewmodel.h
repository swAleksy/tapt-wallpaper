#ifndef DETAILVIEWMODEL_H
#define DETAILVIEWMODEL_H

#include <QObject>
#include <QFutureWatcher>
#include <qqml.h>
#include <QString>
#include <qtypes.h>
#include <QImage>
#include "lutfilterslistmodel.h"

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

public:
    explicit DetailViewModel(QObject *parent = nullptr);

    static DetailViewModel* create(QQmlEngine*, QJSEngine*)
    {
            return new DetailViewModel();
    }

    QString imageUrl()          const { return m_imageUrl; }
    QString imageName()         const { return m_imageName; }
    bool    hasImage()          const { return !m_imageUrl.isEmpty(); }
    qreal   hue()               const { return m_hue; }
    qreal   brightness()        const { return m_brightness; }
    qreal   saturation()        const { return m_saturation; }
    bool    flipped()           const { return m_flipped; }
    int     activeFilterIndex() const { return m_activeFilterIndex; }


    void clear();

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

    void StateReverted();
    void imageLoaded();

private:
    LutFiltersListModel *m_lutFiltersListModel;

    struct ColorState
    {
        qreal hue         = 0.0;
        qreal brightness  = 0.0;
        qreal saturation  = 0.0;
        bool  flipped     = false;
        int   filterIndex = -1;
    };

    QImage m_originalImage;    // wczytany raz, nigdy nie ruszany
    QImage m_lutProcessed;     // wynik LUT, cache — nie przeliczaj przy każdym HSB sliderze
    QFutureWatcher<QImage> m_lutWatcher;

    ColorState m_current;
    ColorState m_committed;

    QString m_imageUrl;
    QString m_imageName;

    qreal m_hue = 0.0, m_brightness = 0.0, m_saturation = 0.0;
    bool  m_flipped = false;
    int   m_activeFilterIndex = -1;
};


#endif // DETAILVIEWMODEL_H
