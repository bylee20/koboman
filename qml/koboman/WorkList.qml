import QtQuick 2.2
import net.xylosper.Mobile 1.0 as X
import QtQuick.Layouts 1.1

Item {
	width: parent.width; height: parent.height;
	ListView {
		id: view; model: BookList
		anchors.fill: parent; //anchors.bottomMargin: scanButton.height
		readonly property real gap: X.Theme.spacing*0.5
		property string selectedRegistrationNumber
		Component {
			id: item
			Rectangle {
				gradient: Gradient {
					GradientStop { position: 0.00; color: Qt.rgba(0.99, 0.99, 0.99, 1.0) }
					GradientStop { position: 0.05; color: Qt.rgba(0.97, 0.97, 0.97, 1.0) }
					GradientStop { position: 0.95; color: Qt.rgba(0.94, 0.94, 0.95, 1.0) }
					GradientStop { position: 1.00; color: Qt.rgba(0.80, 0.80, 0.80, 1.0) }
				}
				width: view.width; height: X.Theme.lineHeight

				RowLayout {
					anchors {
						top: parent.top; topMargin: view.gap
						left: parent.left; leftMargin: view.gap
						right: parent.right; rightMargin: view.gap
					}
					height: X.Utility.dpToPx(20)
					Text {
						font.bold: true; text: registrationNumber
						anchors.verticalCenter: parent.verticalCenter
					}
					Text { text: " - "; anchors.verticalCenter: parent.verticalCenter }
					Text {
						text: callNumber; Layout.fillWidth: true;
						font.pixelSize: X.Theme.textSizeSmall
						anchors.verticalCenter: parent.verticalCenter
					}
					Text {
						Layout.fillWidth: true
						text: status
						font.pixelSize: X.Theme.textSizeMicro
						color: "red"
						anchors.verticalCenter: parent.verticalCenter
						horizontalAlignment: Text.AlignRight
					}
				}

				Text {
					height: X.Utility.dpToPx(20)
					anchors {
						bottom: parent.bottom; bottomMargin: view.gap
						left: parent.left; leftMargin: view.gap
						right: parent.right; rightMargin: view.gap
					}
					textFormat: Text.PlainText
					text: title + ' <' + author + '>'
					verticalAlignment: Text.AlignVCenter
					elide: Text.ElideRight
				}
			}
		}
		delegate: item

	}
	X.ScrollBar {
		scrollable: view
		anchors { bottom: view.bottom; right: view.right; top: view.top }
	}
}
