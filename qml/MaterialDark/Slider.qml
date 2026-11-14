// Material Dark - Qt Quick Controls 2: Slider
// Track 8px (isi primary), knob gelap bercincin primary, halo saat hover/tekan; sama dengan CC_Slider.
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T

T.Slider {
    id: control

    SystemPalette { id: accent; colorGroup: SystemPalette.Active }

    readonly property color primary: accent.highlight
    readonly property color onPrimary: accent.highlightedText
    readonly property color trackColor: Qt.tint(palette.window, Qt.alpha(palette.windowText, 0.15))

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitHandleHeight + topPadding + bottomPadding)
    padding: 6

    handle: Item {
        implicitWidth: 20
        implicitHeight: 20
        x: control.leftPadding + (control.horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))

        Rectangle {
            anchors.fill: parent
            radius: width / 2
            color: Qt.alpha(control.primary, control.pressed ? 0.20 : 0.12)
            visible: control.enabled && (control.hovered || control.pressed)
        }
        Rectangle {
            anchors.centerIn: parent
            width: 16
            height: 16
            radius: 8
            color: control.enabled ? control.onPrimary : Qt.alpha(control.palette.windowText, 0.38)
            border.width: control.enabled ? 2 : 0
            border.color: control.primary
        }
    }

    background: Item {
        implicitWidth: control.horizontal ? 200 : 20
        implicitHeight: control.horizontal ? 20 : 200
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        width: control.horizontal ? control.availableWidth : 8
        height: control.horizontal ? 8 : control.availableHeight

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: control.trackColor
        }
        Rectangle {
            visible: control.enabled
            radius: 4
            color: control.primary
            x: control.horizontal ? 0 : 0
            y: control.horizontal ? 0 : control.visualPosition * parent.height + control.handle.height / 2 - control.handle.height / 2
            width: control.horizontal ? control.visualPosition * parent.width + control.handle.width * (0.5 - control.visualPosition) : parent.width
            height: control.horizontal ? parent.height : parent.height - y
        }
    }
}
