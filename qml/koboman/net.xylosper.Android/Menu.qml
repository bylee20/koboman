import QtQuick 2.1
import QtQuick.Layouts 1.1
import KoboMan 1.0

Popup {
	id: menu

	property list<MenuItem> items
	property real layoutWidth: Utility.dpToPx(220)
	readonly property real gap: Utility.dpToPx(2)
	property real padding: Utility.dpToPx(15)
	property Action title: Action {
		id: titleAction
		visible: text.length > 0
		parent: layout
		color: "#33B5E5"
		interactive: false
		separator.visible: true
		separator.color: "#33B5E5"
		padding: menu.padding
		font.pixelSize: Utility.font.pixelSize*1.5
		iconSize: Utility.font.pixelSize*4
	}

	color: "#EFEFEF"

	contentItem: ColumnLayout {
		id: layout
		width: layoutWidth
		spacing: 0
	}

	onItemsChanged: {
		title.parent = layout
		var len = items.length
		for (var i=0; i<len; ++i) {
			var item = items[i]
			if (item.padding < 0)
				item.padding = Qt.binding(function () { return menu.padding })
			item.parent = layout
			item.triggered.connect(menu.hide)
			item.separator.visible = i < len-1
		}
	}
}
