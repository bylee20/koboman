import QtQuick 2.2
import net.xylosper.Mobile 1.0

Item {
	id: win
	objectName: "App"
	property Item actionBar
	property Item contentItem

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

	MessageBox { id: mbox; onReturned: if (win.messageBox.cb) win.messageBox.cb(result) }

	function messageBox(title, text, buttons, cb) {
		messageBox.cb = cb
		mbox.title.text = title
		mbox.text = text
		if (buttons)
			mbox.buttons = buttons
		mbox.show()
	}

	onActionBarChanged: if (actionBar) actionBar.parent = actionBarContainer
	onContentItemChanged: if (contentItem) contentItem.parent = contentItemContainer
}
