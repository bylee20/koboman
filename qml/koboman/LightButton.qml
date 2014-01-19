import QtQuick 2.0

Item {
	id: button
	property real padding: 20
	property alias text: buttonText.text
	property alias font: buttonText.font
	property bool blink: true
	signal clicked()
	width: buttonText.contentWidth + 2*padding
	height: buttonText.font.pixelSize + 2*padding;
	Rectangle {
		anchors.fill: parent;
		color: mouse.pressed && button.blink ? "#eef" : "#eee"
		Rectangle {
			anchors { fill: parent; margins: 2 }
			color: mouse.pressed && button.blink ? "#33B5E5" : "#ddd"
		}
		Rectangle {
			anchors.bottom: parent.bottom; width: parent.width; height: 2
			color: mouse.pressed && button.blink ? "#0099CC" : "#ccc"
		}
	}
	Text {
		id: buttonText; anchors.centerIn: parent
		verticalAlignment: Text.AlignVCenter; horizontalAlignment: Text.AlignHCenter
	}
	MouseArea { id: mouse; anchors.fill: parent; onClicked: button.clicked() }
}
