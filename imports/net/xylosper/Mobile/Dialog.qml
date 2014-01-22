import QtQuick 2.0
import net.xylosper.Mobile 1.0

TopLevel {
	id: dialog
	property alias title: header.text
	property alias buttons: bbox.buttons
	property Item contentItem: Item {}
	property alias contentWidth: item.width
	property alias contentHeight: item.height
	readonly property alias minimumHeight: item.minimumLength
	signal returned(int result)
	function button(b) { return bbox.button(b); }
	container.item: ItemList {
		id: item
		width: dialog.width*0.75
		height: minimumLength
		interactive: false
		orientation: Qt.Vertical
		separator {
			color: "#575D62"
			thickness: Utility.dp1
		}
		headerItem: Text {
			id: header
			visible: text.length > 0
			ItemList.thickness: font.pixelSize + Utility.dp40
			ItemList.horizontalPadding: Utility.dp20
			verticalAlignment: Text.AlignVCenter
			color: "#33B5E5"
		}
		headerSeparator: ItemListSeparator {
			ItemList.thickness: Utility.dp2
			color: "#33B5E5"
		}
		footerItem: ButtonBox {
			id: bbox
			ItemList.thickness: font.pixelSize + Utility.dp40
			highlight: "#33B5E5"
			orientation: Qt.Horizontal
			fixedItemLength: -1
			buttons: [ ButtonBox.Ok ]
			onTriggered: {
				dialog.hide()
				dialog.returned(button)
			}
		}
		items: [ contentItem ]
	}
	container.position: Qt.point((width-container.item.width)*0.5, (height-container.item.height)*0.5)
}
