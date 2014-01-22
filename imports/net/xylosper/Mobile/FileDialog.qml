import QtQuick 2.2
import net.xylosper.Mobile 1.0

Dialog {
	id: fileDialog
	property alias files: list.texts
	contentHeight: minimumHeight
	buttons: [ButtonBox.Cancel, ButtonBox.Open]
	property alias selectionMode: list.selectionMode
	contentItem: Flickable {
		contentWidth: width
		contentHeight: list.minimumLength
		clip: true
		TextList {
			id: list
			selectionMode: TextList.SingleSelection
			interactive: true
			height: minimumLength
			width: parent.width
		}
		ItemList.thickness: Math.min(contentHeight, fileDialog.height*0.5)
	}
	Binding {
		target: fileDialog.button(ButtonBox.Open)
		property: "enabled"
		value: list.selections.length > 0
	}
	onReturned: {
		if (result == ButtonBox.Open)
			BookList.load(list.selections[0].text)
	}
}
