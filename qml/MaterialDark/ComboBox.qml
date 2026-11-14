// Material Dark - Qt Quick Controls 2: ComboBox
// Sama dengan combobox QtWidgets (materialstyle.cpp): field 10px, outline 1px, fokus/terbuka 2px primary,
// popup berupa menu 12px dengan state layer 10% pada item.
// SPDX-License-Identifier: MIT
import QtQuick
import QtQuick.Templates as T
import QtQuick.Shapes

T.ComboBox {
    id: control

    readonly property color primary: palette.highlight
    readonly property color textColor: palette.text
    // "outline" M3: 60% window + 40% teks; "outline variant": 80% window + 20% teks
    readonly property color outline: Qt.tint(palette.window, Qt.alpha(palette.windowText, 0.4))
    readonly property color outlineVariant: Qt.tint(palette.window, Qt.alpha(palette.windowText, 0.2))
    readonly property color menuSurface: Qt.tint(palette.window, Qt.alpha(palette.windowText, 0.06))
    readonly property int arrowWidth: 28

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             32)

    leftPadding: 10
    rightPadding: arrowWidth + 2
    topPadding: 1
    bottomPadding: 1
    spacing: 0

    delegate: T.ItemDelegate {
        id: item
        required property var model
        required property int index

        width: ListView.view ? ListView.view.width : control.width
        height: 28
        leftPadding: 10
        rightPadding: 10
        topPadding: 0
        bottomPadding: 0
        text: model[control.textRole]
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled

        contentItem: Text {
            text: item.text
            font: control.font
            color: control.palette.windowText
            opacity: item.enabled ? 1.0 : 0.38
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: 10
            color: Qt.alpha(control.palette.windowText, (item.highlighted || item.down) ? 0.10 : 0.0)
        }
    }

    indicator: Shape {
        // panah bawah: segitiga kecil, sama dengan PE_IndicatorArrowDown
        x: control.width - control.arrowWidth - 1 + (control.arrowWidth - width) / 2
        y: (control.height - height) / 2
        width: 8
        height: 4
        preferredRendererType: Shape.CurveRenderer
        opacity: control.enabled ? 0.8 : 0.38
        ShapePath {
            strokeWidth: 0
            fillColor: control.textColor
            startX: 0
            startY: 0
            PathLine { x: 8; y: 0 }
            PathLine { x: 4; y: 4 }
            PathLine { x: 0; y: 0 }
        }
    }

    contentItem: T.TextField {
        implicitWidth: contentWidth
        text: control.editable ? control.editText : control.displayText
        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator
        selectByMouse: control.selectTextByMouse
        font: control.font
        color: control.editable ? control.palette.text : control.palette.buttonText
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        verticalAlignment: Text.AlignVCenter
        opacity: control.enabled ? 1.0 : 0.38
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 32
        radius: 10
        // permukaan tonal tanpa outline saat idle; hover/terbuka menaikkan nada, fokus = cincin primary 2px
        color: {
            if (!control.enabled)
                return Qt.alpha(control.palette.base, 0.6);
            if (control.hovered || control.visualFocus || control.popup.visible)
                return Qt.tint(control.palette.base, Qt.alpha(control.palette.windowText, 0.06));
            return control.palette.base;
        }
        border.width: (control.enabled && (control.visualFocus || control.popup.visible)) ? 2 : 0
        border.color: control.primary
    }

    popup: T.Popup {
        y: control.height + 2
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + topPadding + bottomPadding,
                                 control.Window.height - topMargin - bottomMargin)
        topMargin: 6
        bottomMargin: 6
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            highlightMoveDuration: 0
        }

        background: Rectangle {
            radius: 12
            color: control.menuSurface
            border.width: 1
            border.color: control.outlineVariant
        }
    }
}
