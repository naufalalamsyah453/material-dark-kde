// Material Dark - Qt Quick Controls 2: ItemDelegate
// Seleksi/hover berbentuk pil 8px dengan margin kecil (sama dengan PE_PanelItemViewItem di style Qt).
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ItemDelegate {
    id: control

    SystemPalette { id: accent; colorGroup: SystemPalette.Active }

    readonly property bool selected: highlighted || checked
    readonly property color primary: accent.highlight
    readonly property color contentColor: !enabled ? Qt.alpha(palette.text, 0.38) : (selected ? accent.highlightedText : palette.text)

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             32)
    padding: 8
    leftPadding: 12
    rightPadding: 12
    spacing: 8
    icon.width: 16
    icon.height: 16
    icon.color: contentColor

    contentItem: IconLabel {
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        alignment: control.display === IconLabel.IconOnly || control.display === IconLabel.TextUnderIcon ? Qt.AlignCenter : Qt.AlignLeft | Qt.AlignVCenter
        icon: control.icon
        text: control.text.replace(/&/g, "&&") // "&" literal di daftar, bukan mnemonic
        font: control.font
        color: control.contentColor
    }

    background: Item {
        implicitHeight: 32
        Rectangle {
            x: 3
            y: 1
            width: parent.width - 6
            height: parent.height - 2
            radius: 8
            color: {
                if (!control.enabled)
                    return "transparent";
                if (control.selected)
                    return control.primary;
                return Qt.alpha(control.palette.text, control.down ? 0.12 : control.hovered ? 0.08 : 0.0);
            }
        }
    }
}
