import QtQuick 2.2
import net.xylosper.Mobile 1.0

Dialog {
	id: mbox
	property alias text: message.text
	property alias font: message.font
	contentItem: Text {
		id: message
		ItemList.thickness: implicitHeight
		ItemList.verticalPadding: Utility.dp20
		ItemList.horizontalPadding: Utility.dp20
		wrapMode: Text.WrapAtWordBoundaryOrAnywhere
	}
}
