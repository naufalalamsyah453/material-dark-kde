// Material Dark - Qt Quick Controls 2: Switch
// Mengikuti widgets/switch.svg di Plasma Style: track pil 44x22, knob gelap bercincin primary.
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T

T.Switch {
    id: control

    SystemPalette { id: accent; colorGroup: SystemPalette.Active }

    // palet dari color scheme aktif: highlight = primary, highlightedText = on-primary
    readonly property color primary: accent.highlight
    readonly property color onPrimary: accent.highlightedText

    // di dalam baris terpilih/tertekan (latar pink) aksen dibalik supaya kontrol tidak menyatu dengan latar
    readonly property bool onHighlight: {
        let p = control.parent;
        while (p) {
            if (p.highlighted === true || p.down === true)
                return true;
            p = p.parent;
        }
        return false;
    }
    readonly property color primaryHover: Qt.tint(primary, Qt.rgba(1, 1, 1, 0.45))
    readonly property color trackOff: Qt.tint(palette.window, Qt.rgba(palette.windowText.r, palette.windowText.g, palette.windowText.b, 0.15))

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    padding: 4
    spacing: 8

    indicator: Item {
        implicitWidth: 44
        implicitHeight: 22
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        opacity: control.enabled ? 1.0 : 0.38

        Rectangle {
            id: track
            anchors.fill: parent
            radius: height / 2
            color: control.onHighlight ? (control.checked ? control.onPrimary : Qt.alpha(control.onPrimary, 0.25)) : (control.checked ? control.primary : control.trackOff)
            Behavior on color { ColorAnimation { duration: 120 } }
        }

        Rectangle {
            // ring fokus di sekeliling knob (handle-focus di switch.svg)
            width: 30
            height: 30
            radius: 15
            x: knob.x + knob.width / 2 - width / 2
            y: knob.y + knob.height / 2 - height / 2
            color: "transparent"
            border.width: 2
            border.color: Qt.alpha(control.primary, 0.7)
            visible: control.visualFocus
        }

        Rectangle {
            id: knob
            width: 19
            height: 19
            radius: width / 2
            y: (parent.height - height) / 2
            x: 1.5 + control.visualPosition * (parent.width - parent.height)
            color: control.onHighlight ? control.primary : control.onPrimary
            border.width: 2
            border.color: control.onHighlight ? control.onPrimary : ((control.hovered || control.down) ? control.primaryHover : control.primary)
            Behavior on x {
                enabled: !control.down
                SmoothedAnimation { velocity: 200 }
            }
            Behavior on border.color { ColorAnimation { duration: 120 } }
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
