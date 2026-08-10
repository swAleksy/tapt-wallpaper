#include "viewmodels/timelineviewmodel.h"
#include "models/editstate.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
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

    m_model->addOrUpdate(newItem);
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

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i);
        QString itemId = m_model->data(index, QueueModel::IdRole).toString();
        if (itemId == id) {
            QueueItem updatedItem {
                id,
                m_model->data(index, QueueModel::SourcePathRole).toString(),
                m_model->data(index, QueueModel::NameRole).toString(),
                state
            };

            // Zachowuje harmonogram przed nadpisaniem struktury przez addOrUpdate().
            updatedItem.scheduleStartMin = m_model->data(index, QueueModel::ScheduleStartMinRole).toInt();
            updatedItem.scheduleEndMin = m_model->data(index, QueueModel::ScheduleEndMinRole).toInt();
            updatedItem.weekdayMask = m_model->data(index, QueueModel::WeekdayMaskRole).toInt();

            m_model->addOrUpdate(updatedItem);
            break;
        }
    }
}

void TimelineViewModel::resetItemToDefaults(const QString &id)
{
    EditState defaultState = EditState::identity();

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i);
        QString itemId = m_model->data(index, QueueModel::IdRole).toString();
        if (itemId == id) {
            QueueItem updatedItem {
                id,
                m_model->data(index, QueueModel::SourcePathRole).toString(),
                m_model->data(index, QueueModel::NameRole).toString(),
                defaultState
            };

            // Zachowuje harmonogram przy resetowaniu korekt obrazu.
            updatedItem.scheduleStartMin = m_model->data(index, QueueModel::ScheduleStartMinRole).toInt();
            updatedItem.scheduleEndMin = m_model->data(index, QueueModel::ScheduleEndMinRole).toInt();
            updatedItem.weekdayMask = m_model->data(index, QueueModel::WeekdayMaskRole).toInt();

            m_model->addOrUpdate(updatedItem);
            break;
        }
    }
}

void TimelineViewModel::removeItem(const QString &id)
{
    m_model->remove(id);
}

void TimelineViewModel::clearPlaylist()
{
    m_model->clear();
}

void TimelineViewModel::editItem(const QString &id)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i);
        QString itemId = m_model->data(index, QueueModel::IdRole).toString();
        if (itemId == id) {
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

void TimelineViewModel::moveItem(int from, int to)
{
    m_model->move(from, to);
}

void TimelineViewModel::setCurrentMode(int mode)
{
    if (m_currentMode == mode)
        return;
    m_currentMode = mode;
    emit currentModeChanged();
}

void TimelineViewModel::setLoginOrderMode(int mode)
{
    if (m_loginOrderMode == mode)
        return;
    m_loginOrderMode = mode;
    emit loginOrderModeChanged();
}

void TimelineViewModel::setTimerOrderMode(int mode)
{
    if (m_timerOrderMode == mode)
        return;
    m_timerOrderMode = mode;
    emit timerOrderModeChanged();
}

void TimelineViewModel::setTimerIntervalValue(int value)
{
    if (m_timerIntervalValue == value)
        return;
    m_timerIntervalValue = value;
    emit timerIntervalValueChanged();
}

void TimelineViewModel::setTimerIntervalUnit(int unit)
{
    if (m_timerIntervalUnit == unit)
        return;
    m_timerIntervalUnit = unit;
    emit timerIntervalUnitChanged();
}

bool TimelineViewModel::exportPlaylist(const QString &path) const
{
    QJsonObject root;
    root["mode"] = m_currentMode;

    // Konfiguracja wszystkich trybów zapisywana bezwarunkowo.
    QJsonObject loginSettings;
    loginSettings["orderMode"] = m_loginOrderMode;
    root["whenLoggingIn"] = loginSettings;

    QJsonObject timerSettings;
    timerSettings["orderMode"] = m_timerOrderMode;
    timerSettings["intervalValue"] = m_timerIntervalValue;
    timerSettings["intervalUnit"] = m_timerIntervalUnit;
    root["onATimer"] = timerSettings;

    QJsonArray queue;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        const QModelIndex index = m_model->index(i);

        QJsonObject obj;
        obj["id"] = m_model->data(index, QueueModel::IdRole).toString();
        obj["order"] = i;
        obj["sourcePath"] = m_model->data(index, QueueModel::SourcePathRole).toString();
        // Placeholder do czasu podłączenia renderingu tapet.
        obj["exportedPath"] = m_model->data(index, QueueModel::ExportedPathRole).toString();
        obj["name"] = m_model->data(index, QueueModel::NameRole).toString();

        QJsonObject edit;
        edit["hue"] = m_model->data(index, QueueModel::HueRole).toDouble();
        edit["brightness"] = m_model->data(index, QueueModel::BrightnessRole).toDouble();
        edit["saturation"] = m_model->data(index, QueueModel::SaturationRole).toDouble();
        edit["flipped"] = m_model->data(index, QueueModel::FlippedRole).toBool();
        edit["lutPath"] = m_model->data(index, QueueModel::LutPathRole).toString();
        obj["edit"] = edit;

        obj["scheduleStartMin"] = m_model->data(index, QueueModel::ScheduleStartMinRole).toInt();
        obj["scheduleEndMin"] = m_model->data(index, QueueModel::ScheduleEndMinRole).toInt();
        obj["weekdayMask"] = m_model->data(index, QueueModel::WeekdayMaskRole).toInt();

        queue.append(obj);
    }
    root["queue"] = queue;

    const QJsonDocument doc(root);

    // Tworzy katalog docelowy, jeśli nie istnieje.
    const QDir dir = QFileInfo(path).dir();
    if (!dir.exists() && !dir.mkpath("."))
        return false;

    // Zapis atomowy (QSaveFile) chroni przed odczytem niepełnego pliku przez watchera.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}
