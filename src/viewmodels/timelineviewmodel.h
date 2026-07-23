#ifndef TIMELINEVIEWMODEL_H
#define TIMELINEVIEWMODEL_H

#include "models/playlistmodel.h"
#include <QObject>
#include <qqml.h>

class TimelineViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(PlaylistModel* playlistModel READ playlistModel CONSTANT)

public:
    static TimelineViewModel* create(QQmlEngine*, QJSEngine*) { return new TimelineViewModel(); }
    PlaylistModel* playlistModel() const { return m_model; }

    Q_INVOKABLE QString addItem(
        const QString& sourcePath,
        const QString& name,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath); // Returns new ID.

    Q_INVOKABLE void updateItem(
        const QString& id,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath);

    Q_INVOKABLE void resetItemToDefaults(const QString& id);
    Q_INVOKABLE void removeItem(const QString& id);
    Q_INVOKABLE void editItem(const QString& id); // Click in Timeline -> open in DetailView.

signals:
    void itemRequestedForEditing(
        const QString& id,
        const QString& sourcePath,
        const QString& name,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath);

private:
    PlaylistModel* m_model;
};

#endif // TIMELINEVIEWMODEL_H
