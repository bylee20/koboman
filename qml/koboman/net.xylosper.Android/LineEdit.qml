import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import KoboMan 1.0

TextField {
	id: field
	style:  TextFieldStyle {
		background: Rectangle {
			implicitWidth: field.width
			implicitHeight: field.font.pixelSize+20
			color: "white"
			Rectangle {
				anchors.bottom: parent.bottom
				color: field.activeFocus ? "#33B5E5" : "#ccc"
				width: parent.width
				height: Utility.dpToPx(4)
				Rectangle {
					readonly property real pad: Utility.dpToPx(2)
					anchors.top: parent.top
					anchors.horizontalCenter: parent.horizontalCenter
					width: parent.width - pad*2;
					height: parent.height - pad;
					color: "white"
				}
			}
		}
		placeholderTextColor: "#ccc"
	}
}
