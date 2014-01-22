import QtQuick 2.2
import net.xylosper.Mobile 1.0

TopLevel {
	id: busy
	autohide: false
	shade: 0.6
	property alias text: msg.text
	BusyIndicator {
		id: indicator
		width: Utility.dpToPx(150); height: width
		anchors.centerIn: parent
	}
	Text {
		id: msg
		color: Theme.highlight
		anchors {
			top: indicator.bottom
			bottom: parent.bottom
			left: parent.left
			right: parent.right
			margins: Theme.padding*2
		}
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
		font.pixelSize: Theme.textSizeLarge
		wrapMode: Text.WrapAtWordBoundaryOrAnywhere
	}
}
