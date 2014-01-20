import QtQuick 2.2
import net.xylosper.Mobile 1.0

Rectangle {
	width: parent.width; height: parent.height; color: Theme.background
	Rectangle {
		width: parent.width; height: Theme.separatorThick
		anchors.bottom: parent.bottom
		color: Theme.separatorColor
	}

	ItemList {
		anchors.fill: parent
		color: "#9900ccff"
	}


//	property list<Item> buttons
//	property Item contentItem
//	onButtonsChanged: {
//		for (var i=0; i<buttons.length; ++i)
//			buttons[i].parent = layout
//	}
//	onContentItemChanged: if (contentItem) contentItem.parent = container
//	Rectangle {
//		anchors.bottom: parent.bottom; width: parent.width; height: parent.height*0.0375; color: "#C2C2C2"
//	}
//	RowLayout {
//		id: layout; anchors.fill: parent
//		Item { id: container; Layout.fillWidth: true; Layout.fillHeight: true }
//	}
}
