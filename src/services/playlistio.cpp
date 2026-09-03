#include "playlistio.h"
#include "models/monitorplayliststate.h"
#include "models/playlistenums.h"
#include "models/queuemodel.h"
#include "models/queueitem.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace PlaylistIO {

namespace {

QJsonObject serializeMonitorState(const MonitorPlaylistState *state)
{
    QJsonObject obj;
    obj["mode"] = PlaylistEnums::toString(state->currentMode());

    QJsonObject loginSettings;
    loginSettings["orderMode"] = PlaylistEnums::toString(state->loginOrderMode());
    obj["whenLoggingIn"] = loginSettings;

    QJsonObject timerSettings;
    timerSettings["orderMode"] = PlaylistEnums::toString(state->timerOrderMode());
    timerSettings["intervalValue"] = state->timerIntervalValue();
    timerSettings["intervalUnit"] = PlaylistEnums::toString(state->timerIntervalUnit());
    obj["onATimer"] = timerSettings;

    QJsonArray queue;
    const QueueModel *model = state->queueModel();
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i);
        QJsonObject item;
        item["id"] = model->data(index, QueueModel::IdRole).toString();
        item["sourcePath"] = model->data(index, QueueModel::SourcePathRole).toString();
        item["exportedPath"] = model->data(index, QueueModel::ExportedPathRole).toString();
        item["name"] = model->data(index, QueueModel::NameRole).toString();

        QJsonObject edit;
        edit["hue"] = model->data(index, QueueModel::HueRole).toDouble();
        edit["brightness"] = model->data(index, QueueModel::BrightnessRole).toDouble();
        edit["saturation"] = model->data(index, QueueModel::SaturationRole).toDouble();
        edit["flipped"] = model->data(index, QueueModel::FlippedRole).toBool();
        edit["lutPath"] = model->data(index, QueueModel::LutPathRole).toString();
        item["edit"] = edit;

        item["scheduleStartMin"] = model->data(index, QueueModel::ScheduleStartMinRole).toInt();
        item["scheduleEndMin"] = model->data(index, QueueModel::ScheduleEndMinRole).toInt();
        item["weekdayMask"] = model->data(index, QueueModel::WeekdayMaskRole).toInt();

        queue.append(item);
    }
    obj["queue"] = queue;
    return obj;
}

struct ParsedQueueItem {
    QString id;
    QString sourcePath;
    QString exportedPath;
    QString name;
    EditState edit;
    int scheduleStartMin = 0;
    int scheduleEndMin = 0;
    int weekdayMask = 0;
};

struct ParsedMonitorState {
    PlaylistEnums::Mode mode = PlaylistEnums::Mode::TimeOfDay;
    PlaylistEnums::OrderMode loginOrderMode = PlaylistEnums::OrderMode::Random;
    PlaylistEnums::OrderMode timerOrderMode = PlaylistEnums::OrderMode::Random;
    int timerIntervalValue = 30;
    PlaylistEnums::IntervalUnit timerIntervalUnit = PlaylistEnums::IntervalUnit::Minutes;
    QList<ParsedQueueItem> items;
};

bool parseMonitorState(const QJsonObject &obj, ParsedMonitorState &out)
{
    // Ciało z Twojego TimelineViewModel.cpp pozostało całkowicie takie samo.
    if (!obj["mode"].isString() || !PlaylistEnums::fromString(obj["mode"].toString(), out.mode)) return false;

    if (!obj["whenLoggingIn"].isObject()) return false;
    const QJsonObject login = obj["whenLoggingIn"].toObject();
    if (!login["orderMode"].isString() || !PlaylistEnums::fromString(login["orderMode"].toString(), out.loginOrderMode)) return false;

    if (!obj["onATimer"].isObject()) return false;
    const QJsonObject timer = obj["onATimer"].toObject();
    if (!timer["orderMode"].isString() || !PlaylistEnums::fromString(timer["orderMode"].toString(), out.timerOrderMode)) return false;
    if (!timer["intervalValue"].isDouble()) return false;
    out.timerIntervalValue = timer["intervalValue"].toInt();
    if (!timer["intervalUnit"].isString() || !PlaylistEnums::fromString(timer["intervalUnit"].toString(), out.timerIntervalUnit)) return false;

    if (!obj["queue"].isArray()) return false;

    for (const QJsonValue &v : obj["queue"].toArray()) {
        if (!v.isObject()) return false;
        const QJsonObject item = v.toObject();

        if (!item["id"].isString() || item["id"].toString().isEmpty()) return false;
        if (!item["edit"].isObject()) return false;
        const QJsonObject edit = item["edit"].toObject();

        ParsedQueueItem qi;
        qi.id = item["id"].toString();
        qi.sourcePath = item["sourcePath"].toString();
        qi.exportedPath = item["exportedPath"].toString();
        qi.name = item["name"].toString();
        qi.edit.hue = edit["hue"].toDouble();
        qi.edit.brightness = edit["brightness"].toDouble();
        qi.edit.saturation = edit["saturation"].toDouble();
        qi.edit.flipped = edit["flipped"].toBool();
        qi.edit.lutPath = edit["lutPath"].toString();
        qi.scheduleStartMin = item["scheduleStartMin"].toInt();
        qi.scheduleEndMin = item["scheduleEndMin"].toInt();
        qi.weekdayMask = item["weekdayMask"].toInt();

        out.items.append(qi);
    }
    return true;
}

} // namespace

bool exportPlaylist(const QString &path, const QMap<QString, MonitorPlaylistState*> &monitorStates)
{
    QJsonObject monitors;
    for (auto it = monitorStates.constBegin(); it != monitorStates.constEnd(); ++it)
        monitors[it.key()] = serializeMonitorState(it.value());

    QJsonObject root;
    root["monitors"] = monitors;

    const QJsonDocument doc(root);
    const QDir dir = QFileInfo(path).dir();
    if (!dir.exists() && !dir.mkpath("."))
        return false;

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}

bool importPlaylist(const QString &path, QMap<QString, MonitorPlaylistState*> &monitorStates, QObject *parentForNew)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    if (!doc.object()["monitors"].isObject())
        return false;
    const QJsonObject monitors = doc.object()["monitors"].toObject();

    QMap<QString, ParsedMonitorState> parsed;
    for (auto it = monitors.constBegin(); it != monitors.constEnd(); ++it) {
        if (!it.value().isObject()) return false;

        ParsedMonitorState state;
        if (!parseMonitorState(it.value().toObject(), state)) return false;
        parsed.insert(it.key(), state);
    }

    for (auto it = parsed.constBegin(); it != parsed.constEnd(); ++it) {
        MonitorPlaylistState *monitorState = monitorStates.value(it.key());
        if (!monitorState) {
            monitorState = new MonitorPlaylistState(parentForNew);
            monitorStates.insert(it.key(), monitorState);
        }

        const ParsedMonitorState &state = it.value();
        monitorState->setCurrentMode(state.mode);
        monitorState->setLoginOrderMode(state.loginOrderMode);
        monitorState->setTimerOrderMode(state.timerOrderMode);
        monitorState->setTimerIntervalValue(state.timerIntervalValue);
        monitorState->setTimerIntervalUnit(state.timerIntervalUnit);

        QueueModel *model = monitorState->queueModel();
        model->clear();
        for (const ParsedQueueItem &qi : state.items) {
            QueueItem item { qi.id, qi.sourcePath, qi.name, qi.edit };
            item.exportedPath = qi.exportedPath;
            item.scheduleStartMin = qi.scheduleStartMin;
            item.scheduleEndMin = qi.scheduleEndMin;
            item.weekdayMask = qi.weekdayMask;
            model->addOrUpdate(item);
        }
    }

    return true;
}

QString defaultPlaylistPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + QStringLiteral("/playlist.json");
}

} // namespace PlaylistIO
