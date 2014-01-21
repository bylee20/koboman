import QtQuick 2.2
import net.xylosper.Mobile 1.0

TopLevel {
	id: dialog
	property alias contentWidth: item.width
	property alias contentHeight: item.height
	readonly property alias minimumHeight: item.minimumLength
	property alias actions: item.items
	signal returned(int result)
	shade: 0.0
	container.color: Theme.background
	container.item: ItemList {
		id: item
		fixedItemLength: Theme.lineHeight
		orientation: Qt.Vertical
		width: Utility.dpToPx(220)
		height: item.minimumLength
		onClicked: { dialog.hide() }
	}
}
