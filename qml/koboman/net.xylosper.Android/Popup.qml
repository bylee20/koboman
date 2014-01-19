import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import KoboMan 1.0

Item {
	id: popup; z: 1e100; parent: Utility.root; anchors.fill: parent
	visible: false; focus: visible && cancelable
	Keys.onPressed: event.accepted = true
	Keys.onReleased: {
		if (event.key === Qt.Key_Back) {
			event.accepted = true;
			hide()
		}
	}
	onParentChanged: {
		if (parent !== Utility.root)
			parent = Utility.root
	}
	property bool shown: false
	property alias shadow: shadowImage.visible
	property real margin: Utility.dpToPx(10)
	property alias color: box.color
	property bool cancelable: true
	property Item contentItem: Item {}
	property Item attach
	property real shade: 0.0
	property point origin: Qt.point(0, 0)
	property point pos: attach ? attachedPosition(attach.x, attach.y, attach.width, attach.height) : center
	readonly property point center: Qt.point((popup.width - box.width)*0.5, (popup.height - box.height)*0.5);

	function attachedPosition(bx, by, bw, bh) {
		var p = attach.mapToItem(null, 0, bh)
		var r = p.x + box.width + popup.margin
		var b = p.y + box.height + popup.margin
		if (r > popup.width)
			p.x -= r - popup.width
		if (b > popup.height)
			p.y -= b - popup.height
		return Qt.point(p.x, p.y)
	}

	function show() {
		box.shown = true
		popup.visible = true;
		background.visible = true
	}
	function hide() {
		box.shown = false
		popup.visible = false
	}
	onContentItemChanged: { contentItem.parent = box; contentItem.x = contentItem.y = 0 }

	MouseArea {
		anchors.fill: parent
		visible: popup.cancelable
		onPressed: background.visible = false
		onReleased: popup.visible = false
	}

	Rectangle {
		id: background
		anchors.fill: parent
		color: Qt.rgba(0.0, 0.0, 0.0, popup.shade)

		BorderImage {
			id: shadowImage
			property real gap: Utility.dpToPx(7)
			width: (box.width + gap*2)/scale
			height: (box.height + gap*2)/scale
			transformOrigin: Item.Center
			anchors.centerIn: box
			anchors.verticalCenterOffset: Utility.dpToPx(1)
			border { left: 32; top: 32; right: 32; bottom: 32 }
			horizontalTileMode: BorderImage.Stretch
			verticalTileMode: BorderImage.Stretch
			scale: (Utility.dpi/160)/2
			source: "rectshadow.png"
		}

		MouseArea {
			anchors.fill: box
		}

		Rectangle {
			id: box; color: "#D7D7D7"
			x: pos.x; y: pos.y; width: contentItem.width; height: contentItem.height
			property bool shown: false
//			transform: Scale { origin.x: 0; origin.y: 0; xScale: 0.0; yScale: xScale}
//			states: [State {
//				name: "SHOWN"; when: box.shown
//				PropertyChanges { target: box.transform; xScale: 1.0 }
//			}, State {
//				name: "HIDDEN"; when: !box.shown
//				PropertyChanges { target: contextMenu; opacity: 0.0; visible: false }
//				PropertyChanges { target: menuContainer; scale: 0.0 }
//			}]
		}
	}


//	transitions: Transition {
//		from: "HIDDEN"; to: "SHOWN"; reversible: true
//		SequentialAnimation {
//			PropertyAction { property: "visible" }
//			ParallelAnimation {
//				NumberAnimation { target: contextMenu; properties: "opacity"; duration: 50 }
//				NumberAnimation { target: menuContainer; property: "scale"; duration: 50 }
//			}
//		}
//	}
}
