#include "viewmodels/timelineviewmodel.h"
#include "models/editstate.h"
#include <QUuid>
#include <qcoreapplication.h>

TimelineViewModel::TimelineViewModel(QObject *parent)
    : QObject(parent)
    , m_model(new QueueModel(this))
{
}
QString TimelineViewModel::addItem(
    const QString &sourcePath,
    const QString &name,
    qreal hue,
    qreal brightness,
    qreal saturation,
    bool flipped,
    const QString &lutPath)
{
    EditState state {
        hue,
        brightness,
        saturation,
        flipped,
        lutPath
    };


    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QueueItem newItem {id, sourcePath, name, state};

    m_model -> addOrUpdate(newItem);
    return id;

}

void TimelineViewModel::updateItem(
    const QString &id,
    qreal hue,
    qreal brightness,
    qreal saturation,
    bool flipped,
    const QString &lutPath)
{
    EditState state {
        hue,
        brightness,
        saturation,
        flipped,
        lutPath
    };

    // Find the item and update it
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i);
        QString itemId = m_model->data(index, QueueModel::IdRole).toString();
        if (itemId == id) {
            // Create a new QueueItem with updated edit state
            QueueItem updatedItem {
                id,
                m_model->data(index, QueueModel::SourcePathRole).toString(),
                m_model->data(index, QueueModel::NameRole).toString(),
                state
            };

            m_model->addOrUpdate(updatedItem);
            break;
        }
    }
}

void TimelineViewModel::resetItemToDefaults(const QString &id)
{
    // Reset to default edit state
    EditState defaultState = EditState::identity();

    // Find the item and update it
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i);
        QString itemId = m_model->data(index, QueueModel::IdRole).toString();
        if (itemId == id) {
            // Create a new QueueItem with default edit state
            QueueItem updatedItem {
                id,
                m_model->data(index, QueueModel::SourcePathRole).toString(),
                m_model->data(index, QueueModel::NameRole).toString(),
                defaultState
            };

            m_model->addOrUpdate(updatedItem);
            break;
        }
    }
}

void TimelineViewModel::removeItem(const QString &id)
{
    m_model->remove(id);
}

void TimelineViewModel::editItem(const QString &id)
{
    // Find the item to get its details
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i);
        QString itemId = m_model->data(index, QueueModel::IdRole).toString();
        if (itemId == id) {
            // Emit signal with item details for editing
            emit itemRequestedForEditing(
                id,
                m_model->data(index, QueueModel::SourcePathRole).toString(),
                m_model->data(index, QueueModel::NameRole).toString(),
                m_model->data(index, QueueModel::HueRole).toReal(),
                m_model->data(index, QueueModel::BrightnessRole).toReal(),
                m_model->data(index, QueueModel::SaturationRole).toReal(),
                m_model->data(index, QueueModel::FlippedRole).toBool(),
                m_model->data(index, QueueModel::LutPathRole).toString()
            );
            break;
        }
    }
}
