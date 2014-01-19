import QtQuick 2.1
import QtQuick.Layouts 1.1
import KoboMan 1.0

Rectangle {
	id: sep
	readonly property real gap: Utility.dpToPx(1)
	Layout.preferredHeight: gap
	Layout.minimumHeight: gap
	Layout.maximumHeight: gap
	height: gap
	implicitHeight: gap

	Layout.fillWidth: true
	width: parent.width

	color: "#575D62"
}
