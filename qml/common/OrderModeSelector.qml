import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Reusable "Order:" picker (Random / Ordered) shared by the "When logging in"
// and "On a timer" configuration panes.
//
// API mirrors a plain input control (compare SpinBox.value / onValueModified
// used elsewhere in TimelinePanel.qml):
//   - `value`               : current selection (0 = Random, 1 = Ordered)
//   - `valueModified(int)`  : emitted only on a genuine *user* change
//
// The component drives `checkedButton` imperatively from its OWN `value`
// property (Component.onCompleted + onValueChanged), never from an external
// object's signal — so a caller just binds `value:` to the model and there is
// no need for a `Connections` block watching a foreign target.
RowLayout {
    id: root

    // 0 = Random, 1 = Ordered
    property int value: 0
    signal valueModified(int value)

    function syncChecked() {
        group.checkedButton = (root.value === 0) ? randomButton : orderedButton
    }

    onValueChanged: syncChecked()
    Component.onCompleted: syncChecked()

    Label { text: qsTr("Order:") }

    // ButtonGroup owns exclusivity/checked state itself, so we never bind
    // `checked:` on the RadioButtons directly — a literal `checked: expr`
    // binding gets silently detached the first time an auto-exclusive
    // RadioButton writes to `checked` on its own (on click). Instead we drive
    // `checkedButton` imperatively and read it back the same way.
    ButtonGroup {
        id: group
        onCheckedButtonChanged: {
            if (!checkedButton) return
            const newValue = (checkedButton === randomButton) ? 0 : 1
            // Guard against the echo: syncChecked() -> checkedButton change ->
            // this handler. Only report real user-driven changes upward.
            if (newValue !== root.value)
                root.valueModified(newValue)
        }
    }
    RadioButton {
        id: randomButton
        text: qsTr("Random")
        ButtonGroup.group: group
    }
    RadioButton {
        id: orderedButton
        text: qsTr("Ordered")
        ButtonGroup.group: group
    }
}
