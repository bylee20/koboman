import QtQuick 2.2
import net.xylosper.Mobile 1.0

Item {
	id: item; opacity: 0; width: Utility.dpToPx(30)

	property ListView scrollable
	readonly property real gap: Utility.dpToPx(1)

	Item {
		id: box
		anchors {
			top: parent.top
			bottom: parent.bottom
			right: parent.right
			margins: gap
		}
		width: Utility.dpToPx(10)
		Rectangle {
			color: Qt.rgba(0.0, 0.0, 0.0, 0.5)
			radius: Utility.dpToPx(1)
			readonly property real rh: item.scrollable.visibleArea.heightRatio
			readonly property real ry: item.scrollable.visibleArea.yPosition
			readonly property real target: (ry + rh*0.5)*box.height
			readonly property real hMin: Utility.dpToPx(25)
			readonly property real half: Math.max(rh*box.height, hMin)*0.5
			x: gap; y: Math.min(Math.max(0, target - half), box.height)
			width: parent.width - gap*2; height: half + size()
			function size() {
				if (target < half)
					return target
				var dt = box.height - target;
				return (dt < half) ? dt : half;
			}
		}
	}

	states: State {
		name: "visible"
		when: scrollable.movingVertically || mouseArea.moving
		PropertyChanges { target: item; opacity: 1.0 }
	}

	transitions: Transition {
		from: "visible"; to: ""
		NumberAnimation { properties: "opacity"; duration: 600 }
	}

	MouseArea {
		id: mouseArea
		anchors.fill: parent
		property bool moving: false
		function move(y) {
			mouseArea.moving = true
			timer.restart()
			scrollable.positionViewAtIndex((scrollable.count*y/height)-1, ListView.Beginning);
		}
		onPressed: move(mouse.y)
		onPositionChanged: move(mouse.y)
		Timer {
			id: timer
			running: false
			repeat: false
			interval: 3000
			onTriggered: mouseArea.moving = false
		}
	}
}
