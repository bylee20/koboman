import QtQuick 2.1
import KoboMan 1.0

Rectangle {
	id: button
	color: "transparent"
	property bool interactive: true
	property bool enabled: true
	property alias pressed: buttonMouse.pressed
	property Item menu
	onMenuChanged: { if (menu) menu.container.attach = this }

	opacity: enabled ? 1.0 : 0.3
	Rectangle {
		anchors.fill: parent
		color: button.pressed ? "#33B5E5" : "transparent"
	}
	MouseArea {
		id: buttonMouse
		anchors.fill: parent
		visible: button.enabled && interactive
		onClicked: {
			button.triggered()
			if (menu)
				menu.show()
		}
	}
	signal triggered()
}
