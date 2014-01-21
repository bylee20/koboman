import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import net.xylosper.Mobile 1.0 as Mobile
import "net.xylosper.Android" as Android
import KoboMan 1.0

Android.App {
	id: window
	width: 360
	height: 640

	Component.onCompleted: {
		if (BookList.lastWorkingFile.length > 0) {
			messageBox("", "마지막으로 작업한 파일을 불러올까요?", bCancelOk, function(button) {
				if (button === sOk)
					BookList.load(BookList.lastWorkingFile)
			})
		}
	}

	Connections {
		target: BookList
		onLoadingChanged: {
			if (BookList.loading)
				busy.text = qsTr("작업 파일을 불러오는 중입니다...")
			busy.visible = BookList.loading
		}
	}

	Connections {
		target: Library
		onImportingChanged: {
			if (Library.importing)
				busy.text = qsTr("도서 원부 데이터베이스를 갱신하는 중입니다...")
			busy.visible = Library.importing
		}
	}

//	Android.Dialog {
//		id: fileDialog
//		innerItemPadding: 0
//		innerItem: TableView {
//			TableViewColumn{ role: "file"; title: "Title" }
//			model: ListModel {
//				id: fileListModel
//			}
//		 }
//	}

	actionBar: Mobile.ActionBar {
		actions: [
			Mobile.Action {
				enabled: scanner.visible
				icon: scanner.torch ? "qrc:///icon/ic_action_flash_off.png"
									: "qrc:///icon/ic_action_flash_on.png"
				onTriggered: scanner.torch = !scanner.torch

			},
			Mobile.Action {
				enabled: !scanner.scanning
				icon: "qrc:///icon/ic_action_camera.png"
				onTriggered: {
					scanner.visible = true
					scanner.scanning = true
				}

			},
			Mobile.Action {
				id: over
				icon: "qrc:///icon/ic_action_overflow.png"
				menu: Mobile.Menu {
					actions: [
						Mobile.Action {
							text: qsTr("작업 파일 열기")
							onTriggered: fileDialog.show()
						},
						Mobile.Action {
							text: qsTr("도서 원부 불러오기")
							onTriggered: Library.import_()
						}
					]
				}

			}
		]
	}/*Android.ActionBar {
		contentItem: ColumnLayout {
			anchors.fill: parent
			Text {
				text: qsTr("총 %1개 기록됨").arg(BookList.count)
			}
			Text {
				id: msgText
				Connections {
					target: BookList
					onMessageNotified: { msgText.text = message }
				}
				font.pixelSize: Utility.font.pixelSize*0.8
			}
		}

		buttons: [
			Android.ActionBarButton {
				enabled: scanner.visible
				icon: scanner.torch ? "qrc:///icon/ic_action_flash_off.png"
									: "qrc:///icon/ic_action_flash_on.png"
				onTriggered: scanner.torch = !scanner.torch
			},
			Android.ActionBarButton {
				enabled: !scanner.scanning
				icon: "qrc:///icon/ic_action_camera.png"
				onTriggered: {
					scanner.visible = true
					scanner.scanning = true
				}
			},
			Android.ActionBarButton {
				id: over
				icon: "qrc:///icon/ic_action_overflow.png"
				menu: Mobile.Menu {
					actions: [
						Mobile.Action {
							text: qsTr("작업 파일 열기")
							onTriggered: fileDialog.show()
						},
						Mobile.Action {
							text: qsTr("도서 원부 불러오기")
							onTriggered: Library.import_()
						}
					]
				}

			}
		]
	}*/

	contentItem: Item {
		anchors.fill: parent; //anchors.bottomMargin: scanButton.height
		ListView {
			id: view; anchors.fill: parent; model: BookList
			readonly property real gap: Utility.dpToPx(5)
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
					width: view.width; height: Utility.dpToPx(55)

					RowLayout {
						anchors {
							top: parent.top; topMargin: view.gap
							left: parent.left; leftMargin: view.gap
							right: parent.right; rightMargin: view.gap
						}
						height: Utility.dpToPx(20)
						Text {
							font.bold: true; text: registrationNumber
							anchors.verticalCenter: parent.verticalCenter
						}
						Text { text: " - "; anchors.verticalCenter: parent.verticalCenter }
						Text {
							text: callNumber; Layout.fillWidth: true;
							font.pixelSize: Utility.font.pixelSize*0.95
							anchors.verticalCenter: parent.verticalCenter
						}
						Text {
							Layout.fillWidth: true
							text: status
							font.pixelSize: Utility.font.pixelSize*0.9
							color: "red"
							anchors.verticalCenter: parent.verticalCenter
							horizontalAlignment: Text.AlignRight
						}
					}

					Text {
						height: Utility.dpToPx(20)
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
//			focus: true
//			Keys.onReleased: {
//				if (event.key === Qt.Key_Back)
//			}

//			Rectangle {
//				anchors.fill: parent
//				color:"white"
//				Mobile.ItemList {
//					anchors.fill: parent
//					verticalPadding: 20
//					orientation: Qt.Horizontal
//					headerItem: Text {
//						text: "header"
//						Mobile.ItemList.thickness: 100
//						Mobile.ItemList.interactive: true
//					}
//					headerSeparator: Mobile.ItemListSeparator {
//						color: "blue"
//						thickness: 20
//					}
//					separator {
//						color: "green"
//					}
//					onClicked: console.log(item.text, item.Mobile.ItemList.index)

//					items: [
//						Text {
//							color:"green"
//							Mobile.ItemList.thickness: -1
//							Mobile.ItemList.color: Qt.rgba(1.0, 0, 0, 0)
//							text:"aaa"
//						},
//						Rectangle {
//							Mobile.ItemList.verticalPadding: 50
//							color:"black"
//							implicitHeight: 100
//						}

//					]
//				}
//			}
		}

		Android.ScrollBar {
			scrollable: view
			anchors { bottom: view.bottom; right: view.right; top: view.top }
		}

		BarcodeScanner {
			id: scanner; anchors.fill: parent
			visible: false; focus: visible; device: 0
			onVisibleChanged: scanner.scanning = visible
			Keys.onReleased: {
				if (event.key === Qt.Key_Back) {
					visible = false
					event.accepted = true
				}
			}

			onBarcodesChanged: {
				for (var i=0; i<barcodes.length; ++i) {
					if (BookList.append(barcodes[i]))
							break
				}
			}
		}


	}


}
