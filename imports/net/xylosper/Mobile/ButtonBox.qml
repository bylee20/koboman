import QtQuick 2.0
import net.xylosper.Mobile 1.0

TextList {
	orientation: Qt.Horizontal
	font.pixelSize: 50
	height: font.pixelSize + Utility.dpToPx(4)
	width: 300
	texts: ["test", "test2"]
//	list: [
//		Text {
//			id: ref
//			ItemList.thickness: font.pixelSize + Utility.dpToPx(4)
//			ItemList.interactive: true
//			verticalAlignment: Text.AlignVCenter
//			horizontalAlignment: Text.AlignHCenter
//		},
//		Text {
//			font: ref.font
//			ItemList.thickness: font.pixelSize + Utility.dpToPx(4)
//			ItemList.interactive: true
//			verticalAlignment: Text.AlignVCenter
//			horizontalAlignment: Text.AlignHCenter
//		}
//	]
}

//import QtQuick 2.2
//import KoboMan 1.0
//import QtQuick.Layouts 1.1

//MenuItem {
//	id: action
//	property alias text: text.text
//	property alias color: text.color
//	property alias font: text.font
//	property alias verticalAlignment: text.verticalAlignment
//	property alias horizontalAlignment: text.horizontalAlignment
//	property alias wrapMode: text.wrapMode
//	property string icon: ""
//	property real iconSize: 0
//	Layout.fillWidth: true
//	interactive: true
//	contentItem: Item {
//		id: item
//		readonly property real textHeight: text.font.pixelSize + Utility.dpToPx(4)
//		implicitHeight: image.visible && action.iconSize > textHeight ? action.iconSize : textHeight
//		Image {
//			id: image; visible: icon.length > 0
//			source: action.icon
//			width: height*sourceSize.width/sourceSize.height
//			height: iconSize
//		}
//		Text {
//			id: text
//			anchors { top: parent.top; left: image.right; bottom: parent.bottom; right: parent.right }
//			verticalAlignment: Text.AlignVCenter
//			wrapMode: Text.WrapAtWordBoundaryOrAnywhere
//		}
//	}
//}


//	MenuItem {
//		id: bbox
//		interactive: false
//		padding: 0
//		readonly property real pad: Utility.font.pixelSize
//		readonly property int count: Math.min(buttons.length, 3)
//		readonly property real buttonWidth: (width-ref.gap*(count-1))/count
//		contentItem: RowLayout {
//			width: bbox.width
//			spacing: 0
//			Action {
//				id: first
//				text: mbox.buttons.length > 0 ? mbox.buttons[0] : ""
//				width: bbox.buttonWidth
//				horizontalAlignment: Text.AlignHCenter
//				padding: bbox.pad
//				visible: text.length > 0
//				onTriggered: mbox.triggered(0, text)
//			}
//			Rectangle { width: ref.gap; height: parent.height; color: ref.color; visible: second.visible }
//			Action {
//				id: second
//				text: mbox.buttons.length > 1 ? mbox.buttons[1] : ""
//				width: bbox.buttonWidth
//				horizontalAlignment: Text.AlignHCenter
//				padding: bbox.pad
//				visible: text.length > 0
//				onTriggered: mbox.triggered(1, text)
//			}
//			Rectangle { width: ref.gap; height: parent.height; color: ref.color; visible: third.visible }
//			Action {
//				id: third
//				text: mbox.buttons.length > 2 ? mbox.buttons[2] : ""
//				width: bbox.buttonWidth
//				horizontalAlignment: Text.AlignHCenter
//				padding: bbox.pad
//				visible: text.length > 0
//				onTriggered: mbox.triggered(2, text)
//			}
//		}
