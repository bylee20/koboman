import QtQuick 2.2
import net.xylosper.Mobile 1.0

Item {
	id: action
	property alias text: txt.text
	property alias icon: img.source
	property alias iconSize: img.height
	property int orientation: Qt.Horizontal
	ItemList.thickness: txt
	signal triggered()
	Image {
		id: img
		width: height
		height: Theme.iconSize
	}
	Text {
		id: txt
	}
}
