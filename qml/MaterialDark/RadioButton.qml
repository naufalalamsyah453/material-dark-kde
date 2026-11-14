// Material Dark - Qt Quick Controls 2: RadioButton (sama dengan PE_IndicatorRadioButton)
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T

T.RadioButton {
    id: control

    SystemPalette { id: accent; colorGroup: SystemPalette.Active }

    readonly property color primary: accent.highlight
    readonly property color outline: Qt.tint(palette.window, Qt.alpha(palette.windowText, 0.4))

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    padding: 4
    spacing: 8

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        radius: width / 2
        color: "transparent"
        border.width: 2
        border.color: !control.enabled ? Qt.alpha(control.palette.windowText, 0.38)
                      : control.checked ? control.primary
                      : (control.hovered ? control.palette.windowText : control.outline)

        Rectangle {
            width: 8
            height: 8
            radius: 4
            anchors.centerIn: parent
            visible: control.checked
            color: control.enabled ? control.primary : Qt.alpha(control.palette.windowText, 0.38)
        }
    }

    contentItem: Text {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0
        text: control.text
        font: control.font
        color: control.palette.windowText
        opacity: control.enabled ? 1.0 : 0.38
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
