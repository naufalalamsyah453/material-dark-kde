// Material Dark - Qt Quick Controls 2: SpinBox ( - nilai + ), field tonal 10px, fokus cincin primary 2px
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T

T.SpinBox {
    id: control

    SystemPalette { id: accent; colorGroup: SystemPalette.Active }

    readonly property color primary: accent.highlight

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentItem.implicitWidth + 2 * 40)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             32)
    padding: 0
    leftPadding: 40
    rightPadding: 40
    spacing: 0
    editable: true
    inputMethodHints: Qt.ImhFormattedNumbersOnly

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    contentItem: TextInput {
        text: control.displayText
        font: control.font
        color: control.palette.text
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        opacity: control.enabled ? 1.0 : 0.38
    }

    up.indicator: Rectangle {
        x: control.mirrored ? 0 : control.width - width
        width: 40
        height: control.height
        radius: 10
        color: Qt.alpha(control.palette.text, !control.up.pressed && !control.up.hovered ? 0 : control.up.pressed ? 0.10 : 0.08)
        opacity: control.enabled && control.value < control.to ? 1.0 : 0.38
        Rectangle { anchors.centerIn: parent; width: 12; height: 2; radius: 1; color: control.palette.text }
        Rectangle { anchors.centerIn: parent; width: 2; height: 12; radius: 1; color: control.palette.text }
    }

    down.indicator: Rectangle {
        x: control.mirrored ? control.width - width : 0
        width: 40
        height: control.height
        radius: 10
        color: Qt.alpha(control.palette.text, !control.down.pressed && !control.down.hovered ? 0 : control.down.pressed ? 0.10 : 0.08)
        opacity: control.enabled && control.value > control.from ? 1.0 : 0.38
        Rectangle { anchors.centerIn: parent; width: 12; height: 2; radius: 1; color: control.palette.text }
    }

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: 32
        radius: 10
        color: {
            if (!control.enabled)
                return Qt.alpha(control.palette.base, 0.6);
            if (control.hovered || control.activeFocus)
                return Qt.tint(control.palette.base, Qt.alpha(control.palette.windowText, 0.06));
            return control.palette.base;
        }
        border.width: control.enabled && control.activeFocus ? 2 : 0
        border.color: control.primary
    }
}
