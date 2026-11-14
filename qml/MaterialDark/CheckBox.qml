// Material Dark - Qt Quick Controls 2: CheckBox (sama dengan PE_IndicatorCheckBox di materialstyle.cpp)
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T
import QtQuick.Shapes

T.CheckBox {
    id: control

    SystemPalette { id: accent; colorGroup: SystemPalette.Active }

    readonly property color primary: accent.highlight
    readonly property color onPrimary: accent.highlightedText
    readonly property color outline: Qt.tint(palette.window, Qt.alpha(palette.windowText, 0.4))
    readonly property bool marked: checkState !== Qt.Unchecked

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
        radius: 2
        color: control.marked ? (control.enabled ? control.primary : Qt.alpha(control.palette.windowText, 0.38)) : "transparent"
        border.width: control.marked ? 0 : 2
        border.color: !control.enabled ? Qt.alpha(control.palette.windowText, 0.38)
                      : (control.hovered ? control.palette.windowText : control.outline)

        Shape {
            anchors.fill: parent
            visible: control.marked
            ShapePath {
                strokeWidth: 2
                strokeColor: control.enabled ? control.onPrimary : control.palette.window
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                PathPolyline {
                    path: control.checkState === Qt.PartiallyChecked
                          ? [Qt.point(5, 9), Qt.point(13, 9)]
                          : [Qt.point(4.5, 9.3), Qt.point(7.7, 12.4), Qt.point(13.5, 5.8)]
                }
            }
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
