import QtQuick 2.2
import KoboMan 1.0
import QtQuick.Layouts 1.1

Item {
	id: item
	property bool interactive: false
	property real padding: -1
	property Item contentItem: Item {}
	property Item menu
	property color color: "transparent"
	readonly property real gap: separator.visible ? sep.height : 0
	readonly property real __pad: padding < 0 ? 0 : padding
	property Separator separator: Separator {
		id: sep; parent: item; anchors.bottom: parent.bottom; visible: false
	}
	Layout.fillWidth: true
	Layout.preferredHeight: gap + 2*__pad
		+ (contentItem.implicitHeight ? contentItem.implicitHeight : contentItem.height)

	Rectangle { id: box; anchors.fill: parent; color: mouseArea.pressed ? "#33B5E5" : item.color }

	MouseArea {
		id: mouseArea; anchors.fill: parent; visible: interactive
		onClicked: { item.triggered(); if (menu) menu.hide() }
	}

	Item {
		id: wrapper; anchors { fill: parent; margins: __pad; bottomMargin: __pad + gap }
	}

	onContentItemChanged: {
		contentItem.parent = wrapper
		contentItem.anchors.fill = wrapper
		if (contentItem.triggered)
			contentItem.triggered.connect(item.triggered)
	}

	signal triggered()
}
