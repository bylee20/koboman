import QtQuick 2.1
import KoboMan 1.0

BaseButton {
	id: button
	objectName: "ActionBarButton"
	property alias icon: image.source
	width: height*1.1; height: parent.height
	Rectangle {
		anchors.bottom: parent.bottom; width: parent.width; height: parent.height*0.0375
		color: button.pressed ? "#0099CC" : "transparent"
	}
	Image { id: image; anchors.centerIn: parent; width: Utility.dpToPx(32); height: width }
}
