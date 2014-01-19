import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.1
import KoboMan 1.0
import net.xylosper.Mobile 1.0 as Mobile

Mobile.TopLevel {
	id: busy
	autohide: false
	shade: 0.6
	property alias text: msg.text
	property alias textWidth: msg.width
	BusyIndicator {
		id: ind
		anchors.centerIn: parent
		width: Utility.dpToPx(100); height: width
		running: visible
		style: BusyIndicatorStyle {
			indicator: Image {
				visible: control.running
				source: "busy.png"
				NumberAnimation on rotation {
					running: control.running
					loops: Animation.Infinite
					duration: 1000
					from: 0 ; to: 360
				}
			}
		}
	}

	Text {
		id: msg
		anchors.top: ind.bottom
		anchors.topMargin: Utility.dpToPx(30)
		anchors.horizontalCenter: parent.horizontalCenter
		width: parent.width - Utility.dpToPx(70)
		color: "#33B5E5"
		wrapMode: Text.WrapAtWordBoundaryOrAnywhere
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
	}
}
