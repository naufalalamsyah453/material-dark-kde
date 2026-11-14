// Material Dark - Qt Quick Controls 2: Button
// Sama dengan QPushButton di materialstyle.cpp: pil, tonal (default), terisi primary (highlighted),
// datar/transparan (flat); hover 8%, tekan 10% sebagai state layer.
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.Button {
    id: control

    readonly property color primary: palette.highlight
    readonly property color onPrimary: palette.highlightedText
    readonly property bool filled: highlighted || checked
    readonly property color contentColor: !enabled ? Qt.alpha(palette.buttonText, 0.38) : (filled && !flat ? onPrimary : palette.buttonText)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             36)

    leftPadding: 16
    rightPadding: 16
    topPadding: 2
    bottomPadding: 2
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
        implicitWidth: 80
        implicitHeight: 36
        radius: height / 2
        color: {
            if (!control.enabled)
                return control.flat ? "transparent" : Qt.alpha(control.palette.buttonText, 0.12);
            if (control.filled)
                return control.primary;
            return control.flat ? "transparent" : control.palette.button;
        }

        Rectangle {
            // state layer
            anchors.fill: parent
            radius: parent.radius
            color: Qt.alpha(control.contentColor, control.down ? 0.10 : (control.hovered || control.visualFocus) ? 0.08 : 0.0)
        }
    }
}
