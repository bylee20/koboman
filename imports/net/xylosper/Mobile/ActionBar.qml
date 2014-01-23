import QtQuick 2.2
import net.xylosper.Mobile 1.0

Rectangle {
	id: bar
	width: parent.width; height: Theme.lineHeight; color: Theme.background
	property alias icon: image.source
	Rectangle {
		width: parent.width; height: Theme.separatorThick
		anchors.bottom: parent.bottom
		color: Theme.separatorColor
	}
	property alias actions: actionList.items
	signal upNavigation()
	property alias upVisible: up.visible

	ItemList {
		id: actionList
		height: parent.height
		width: minimumLength
		interactive: true
		orientation: Qt.Horizontal
		fixedItemLength: Theme.lineHeight
		separator.thickness: 0
		anchors.right: parent.right
	}

	Rectangle {
		id: appIcon
		width: Theme.padding + Theme.spacing + Theme.iconSize
		height: parent.height
		Image {
			id: up
			width: height
			height: Theme.iconSize/2
			sourceSize: Qt.size(Theme.iconSize/2, Theme.iconSize/2)
			smooth: true
			anchors.verticalCenter: parent.verticalCenter
			source: "qrc:///icon/ic_action_previous_item.png"
		}

		Image {
			id: image
			width: Theme.iconSize; height: Theme.iconSize
			anchors {
				right: parent.right
				verticalCenter: parent.verticalCenter
				rightMargin: Theme.spacing
			}
		}
		color: mouseArea.pressed ? Theme.highlight : "transparent"
		MouseArea {
			id: mouseArea
			anchors.fill: parent
			onClicked: bar.upNavigation();
			visible: up.visible
		}
	}

	property Item contentItem
	Item {
		id: statusBar
		anchors {
			left: appIcon.right
			right: actionList.left
			top: parent.top
			bottom: parent.bottom
			margins: Theme.spacing/2
		}
	}

	onContentItemChanged: if (contentItem) contentItem.parent = statusBar
}
