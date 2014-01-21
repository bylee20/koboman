import QtQuick 2.2
import net.xylosper.Mobile 1.0

Rectangle {
	id: bar
	width: parent.width; height: Theme.lineHeight; color: Theme.background
	Rectangle {
		width: parent.width; height: Theme.separatorThick
		anchors.bottom: parent.bottom
		color: Theme.separatorColor
	}
	property alias actions: actionList.items

	ItemList {
		id: actionList
		height: parent.height
		width: minimumLength
		orientation: Qt.Horizontal
		fixedItemLength: Theme.lineHeight
		separator.thickness: 0
		anchors.right: parent.right
	}
	property Item contentItem
	onContentItemChanged: if (contentItem) contentItem.parent = bar
}
