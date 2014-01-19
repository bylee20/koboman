import QtQuick 2.2
import KoboMan 1.0
import QtQuick.Layouts 1.1

MenuItem {
	id: action
	property alias text: text.text
	property alias color: text.color
	property alias font: text.font
	property alias verticalAlignment: text.verticalAlignment
	property alias horizontalAlignment: text.horizontalAlignment
	property alias wrapMode: text.wrapMode
	property string icon: ""
	property real iconSize: 0
	Layout.fillWidth: true
	interactive: true
	contentItem: Item {
		id: item
		readonly property real textHeight: text.font.pixelSize + Utility.dpToPx(4)
		implicitHeight: image.visible && action.iconSize > textHeight ? action.iconSize : textHeight
		Image {
			id: image; visible: icon.length > 0
			source: action.icon
			width: height*sourceSize.width/sourceSize.height
			height: iconSize
		}
		Text {
			id: text
			anchors { top: parent.top; left: image.right; bottom: parent.bottom; right: parent.right }
			verticalAlignment: Text.AlignVCenter
			wrapMode: Text.WrapAtWordBoundaryOrAnywhere
		}
	}
}
