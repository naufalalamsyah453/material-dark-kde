// Material Dark - Qt Quick Controls 2: ToolButton
// Datar; berteks = pil seperti Button, hanya ikon = 10px (sama dengan PE_PanelButtonTool).
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ToolButton {
    id: control

    readonly property color contentColor: enabled ? palette.buttonText : Qt.alpha(palette.buttonText, 0.38)
    readonly property bool labelled: text.length > 0 && display !== T.AbstractButton.IconOnly

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    padding: 6
    leftPadding: labelled ? 14 : 6
    rightPadding: labelled ? 14 : 6
    spacing: 8
    icon.width: 16
    icon.height: 16
    icon.color: contentColor

    contentItem: IconLabel {
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        icon: control.icon
        text: control.text
        font: control.font
        color: control.contentColor
    }

    background: Rectangle {
        implicitWidth: 32
        implicitHeight: 32
        radius: control.labelled ? height / 2 : 10
        color: control.checked ? control.palette.button : "transparent"

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Qt.alpha(control.contentColor, control.down ? 0.10 : (control.hovered || control.visualFocus) ? 0.08 : 0.0)
        }
    }
}
