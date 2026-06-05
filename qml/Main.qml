import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    width: 800
    height: 600
    title: qsTr("Tapt")

    // No pageStack — use direct content instead
    SplitView {
        id: mainSplit
        anchors.fill: parent
        orientation: Qt.Vertical   // horizontal divider (P1+P2 | P3)

        // ── Top row: P1 | P2 ─────────────────────────────────────────
        SplitView {
            id: topSplit
            SplitView.fillHeight: true   // takes remaining space above P3
            orientation: Qt.Horizontal  // vertical divider between P1 and P2

            // P1 — left, large pane
            Rectangle {
                id: p1
                SplitView.fillWidth: true
                SplitView.minimumWidth: 200
                color: Kirigami.Theme.backgroundColor

                Gallery {
                    anchors.fill: parent
                }
            }

            // P2 — right pane
            Rectangle {
                id: settingsWindow
                SplitView.preferredWidth: 220
                SplitView.minimumWidth: 120
                color: Kirigami.Theme.alternateBackgroundColor

                // ← drop your P2 component here
                DetailView {
                    anchors.fill: parent
                }
            }
        }

        Rectangle {
            id: timeline
            SplitView.preferredHeight: 120
            SplitView.minimumHeight: 60
            color: Kirigami.Theme.backgroundColor

            // ← drop your P3 component here
            Label {
                anchors.centerIn: parent
                text: "Timeline"
                font.pointSize: 18
            }
        }
    }
}