import QtQuick
import org.kde.taptwallpaper

// Day-of-week schedule track: a thin configuration of the generic
// ScheduleTrack. All tile/divider drawing, drag-and-drop and dividers live in
// ScheduleTrack.qml; here we only inject the weekly axis (7 days) and the
// weekdayMask-based decoding of each row's day range.
ScheduleTrack {
    id: root

    readonly property int dayCount: TimelineViewModel.maxDayOfWeekItems

    unitCount: dayCount
    // At most one image per weekday (7). Extra queue items stay hidden.
    visibleCount: Math.min(TimelineViewModel.queueModel.count, dayCount)
    dragKey: "dayItem"

    // Centered, bold weekday initials.
    labelsModel: [
        qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr("Thu"),
        qsTr("Fri"), qsTr("Sat"), qsTr("Sun")
    ]
    labelHAlign: Text.AlignHCenter
    labelVAlign: Text.AlignVCenter
    labelBold: true

    // weekdayMask -> (startDay, endDay) decoding lives once in C++
    // (QueueModel::scheduleStartDayAt/scheduleEndDayAt). Touching
    // `model.weekdayMask` inside these callbacks does NOT do the math — it only
    // registers a reactive dependency on WeekdayMaskRole so the tile/divider
    // binding refreshes when the mask changes (a bare Q_INVOKABLE call creates
    // no such dependency). Same pattern the time-of-day track uses with
    // model.scheduleStartMin/EndMin.
    startAt: (model, index) => { model.weekdayMask; return TimelineViewModel.queueModel.scheduleStartDayAt(index) }
    endAt: (model, index) => { model.weekdayMask; return TimelineViewModel.queueModel.scheduleEndDayAt(index) }
    moveDivider: (index, value) => TimelineViewModel.moveWeekdayDivider(index, value)
    distribute: () => TimelineViewModel.distributeWeekdaysEvenly()
}
