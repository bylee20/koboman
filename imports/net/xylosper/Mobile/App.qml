import QtQuick 2.2
import net.xylosper.Mobile 1.0

Item {
	id: win
	objectName: "App"
	property Item actionBar
	property Item contentItem
	property Busy busy: Busy {}

	Item {
		id: actionBarContainer
		width: parent.width
		height: actionBar ? Utility.dpToPx(48) : 0
		z: contentItemContainer.z + 1
	}
	Item {
		id: contentItemContainer
		width: parent.width
		anchors.top: actionBarContainer.bottom
		anchors.bottom: parent.bottom
	}

//	Dialog {
//		id: mbox; innerItem: Text { id: message; wrapMode: Text.WrapAtWordBoundaryOrAnywhere }
//		onTriggered: if (win.messageBox.cb) win.messageBox.cb(text)
//	}

	function messageBox(title, text, buttons, cb) {
		messageBox.cb = cb
		mbox.title.text = title
		message.text = text
		if (buttons)
			mbox.buttons = buttons
		mbox.show()
	}
	function iconMessageBox(icon, title, text, buttons, cb) {
		mbox.icon = icon
		messageBox(title, text, buttons, cb)
	}

	readonly property url iInfo: "ic_action_about.png"
	readonly property url iWarning: "ic_action_warning.png"
	readonly property url iError: "ic_action_error.png"
	readonly property string sOk: qsTr("확인")
	readonly property string sCancel: qsTr("취소")
	readonly property var bOk: [sOk]
	readonly property var bCancel: [sCancel]
	readonly property var bCancelOk: [sCancel, sOk]

	onActionBarChanged: if (actionBar) actionBar.parent = actionBarContainer
	onContentItemChanged: if (contentItem) contentItem.parent = contentItemContainer
}
