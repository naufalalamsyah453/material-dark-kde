// Material Dark - Qt Quick Controls 2: TextField (field tonal 10px, fokus = cincin primary 2px)
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.TextField {
    id: control

    SystemPalette { id: accent; colorGroup: SystemPalette.Active }

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding,
                             32)

    leftPadding: 10
    rightPadding: 10
    topPadding: 4
    bottomPadding: 4

    color: palette.text
    placeholderTextColor: palette.placeholderText
    selectionColor: palette.highlight
    selectedTextColor: palette.highlightedText
    verticalAlignment: TextInput.AlignVCenter
    opacity: enabled ? 1.0 : 0.38

    PlaceholderText {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)
        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 160
        implicitHeight: 32
        radius: 10
        color: (control.hovered || control.activeFocus) && control.enabled
               ? Qt.tint(control.palette.base, Qt.alpha(control.palette.windowText, 0.06))
               : control.palette.base
        border.width: control.activeFocus && control.enabled ? 2 : 0
        border.color: accent.highlight
    }
}
