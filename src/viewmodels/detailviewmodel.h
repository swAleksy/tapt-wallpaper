#ifndef DETAILVIEWMODEL_H
#define DETAILVIEWMODEL_H

#include <QObject>
#include <qqml.h>
#include <QString>
#include <QImage>
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

public:
    explicit DetailViewModel(QObject *parent = nullptr);

    static DetailViewModel* create(QQmlEngine*, QJSEngine*)
    {
            return new DetailViewModel();
    }

    QString imageUrl()          const { return m_imageUrl; }
    QString imageName()         const { return m_imageName; }
    bool    hasImage()          const { return !m_imageUrl.isEmpty() && m_originalImageValid; }
    qreal   hue()               const { return m_current.hue; }
    qreal   brightness()        const { return m_current.brightness; }
    qreal   saturation()        const { return m_current.saturation; }
    bool    flipped()           const { return m_current.flipped; }
    int     activeFilterIndex() const { return m_current.activeFilterIndex; }
    LutFiltersListModel* lutFiltersListModel() const { return m_lutFiltersListModel; }


    Q_INVOKABLE void setImage(const QString &url, const QString &name);

    Q_INVOKABLE void applyChanges(qreal hue, qreal brightness, qreal saturation, bool flipped, int filterIndex);
    Q_INVOKABLE void revertChanges();
    Q_INVOKABLE void setAsWallpaper();
    Q_INVOKABLE void addToPlaylist();

    Q_INVOKABLE void loadForEditing(const QString &playlistItemId, const QString &sourcePath,
                                     const QString &name, qreal hue, qreal brightness,
                                     qreal saturation, bool flipped, const QString &lutPath);
    // ustawia m_originalImagePath = sourcePath, m_current = {hue, brightness, ...},
    // zapamiętuje m_editingPlaylistItemId = playlistItemId, emituje imageLoaded()

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

private:

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

    ColorState m_current;

    QString m_imageUrl;
    QString m_imageName;

};


#endif // DETAILVIEWMODEL_H
