import QtQuick 2.2
import net.xylosper.Mobile 1.0

TopLevel {
	id: dialog
	property alias contentWidth: item.width
	property alias contentHeight: item.height
	readonly property alias minimumHeight: item.minimumLength
	property list<Action> actions
	signal returned(int result)
	shade: 0.0
	container.color: Theme.background
	container.item: TextList {
		id: item
		orientation: Qt.Vertical
		width: Utility.dpToPx(220)
		height: item.minimumLength
		onClicked: { actions[item.ItemList.index].triggered(); dialog.hide() }
	}
	onActionsChanged: {
		var texts = []
		for (var i=0; i<actions.length; ++i)
			texts[i] = actions[i].text
		item.texts = texts
	}

}
