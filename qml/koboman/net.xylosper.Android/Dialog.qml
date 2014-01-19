import QtQuick 2.2
import QtQuick.Layouts 1.1
import KoboMan 1.0

ContextMenu {
	id: mbox
	title.text: "Test Text"
	Separator { id: ref; visible: false }
	property var buttons: ["OK"]
	property alias innerItemPadding: inner.padding
	signal triggered(int button, string text)
	property Item innerItem: Item { implicitHeight: 1 }
	items: [
		MenuItem {
			id: inner
			interactive: false
			contentItem: innerItem
		},
		MenuItem {
			id: bbox
			interactive: false
			padding: 0
			readonly property real pad: Utility.font.pixelSize
			readonly property int count: Math.min(buttons.length, 3)
			readonly property real buttonWidth: (width-ref.gap*(count-1))/count
			contentItem: RowLayout {
				width: bbox.width
				spacing: 0
				Action {
					id: first
					text: mbox.buttons.length > 0 ? mbox.buttons[0] : ""
					width: bbox.buttonWidth
					horizontalAlignment: Text.AlignHCenter
					padding: bbox.pad
					visible: text.length > 0
					onTriggered: mbox.triggered(0, text)
				}
				Rectangle { width: ref.gap; height: parent.height; color: ref.color; visible: second.visible }
				Action {
					id: second
					text: mbox.buttons.length > 1 ? mbox.buttons[1] : ""
					width: bbox.buttonWidth
					horizontalAlignment: Text.AlignHCenter
					padding: bbox.pad
					visible: text.length > 0
					onTriggered: mbox.triggered(1, text)
				}
				Rectangle { width: ref.gap; height: parent.height; color: ref.color; visible: third.visible }
				Action {
					id: third
					text: mbox.buttons.length > 2 ? mbox.buttons[2] : ""
					width: bbox.buttonWidth
					horizontalAlignment: Text.AlignHCenter
					padding: bbox.pad
					visible: text.length > 0
					onTriggered: mbox.triggered(2, text)
				}
			}
		}
	]
	onTriggered: hide()
}
