#ifndef TIMELINEVIEWMODEL_H
#define TIMELINEVIEWMODEL_H

#include "models/queuemodel.h"
#include <QObject>
#include <qqml.h>

class TimelineViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QueueModel* queueModel READ queueModel CONSTANT)

public:
    // Standardowy konstruktor z parent = nullptr (bez metody create!)
    explicit TimelineViewModel(QObject *parent = nullptr);

    QueueModel* queueModel() const { return m_model; }

    Q_INVOKABLE QString addItem(
        const QString& sourcePath,
        const QString& name,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath);

    Q_INVOKABLE void updateItem(
        const QString& id,
        qreal hue,
        qreal brightness,
        qreal saturation,
        bool flipped,
        const QString& lutPath);

    Q_INVOKABLE void resetItemToDefaults(const QString& id);
    Q_INVOKABLE void removeItem(const QString& id);
    Q_INVOKABLE void editItem(const QString& id);

    Q_INVOKABLE void moveItem(int from, int to);

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
    QueueModel* m_model;
};

#endif // TIMELINEVIEWMODEL_H
