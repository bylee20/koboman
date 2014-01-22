import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import net.xylosper.Mobile 1.0 as X
import KoboMan 1.0

X.App {
	id: window
	width: 360
	height: 640

	X.Busy {
		id: busy; visible: BookList.loading || Library.importing
		text: BookList.loading ? qsTr("작업 파일을 불러오는 중입니다...")
			: Library.importing ? qsTr("도서 원부 데이터베이스를 갱신하는 중입니다...") : ""
	}

	X.FileDialog { id: fileDialog }

	Component.onCompleted: {
		if (BookList.lastWorkingFile.length > 0) {
			messageBox("", "마지막으로 작업한 파일을 불러올까요?", [X.ButtonBox.Cancel, X.ButtonBox.Ok], function(button) {
				if (button === X.ButtonBox.Ok)
					BookList.load(BookList.lastWorkingFile)
			})
		}
	}

	actionBar: X.ActionBar {
		actions: [
			X.Action {
				enabled: (!scanner.visible && scanner.deviceCount > 0) || (scanner.visible && scanner.deviceCount > 1)
				icon: scanner.visible ? "qrc:///icon/ic_action_switch_camera.png"
									  : "qrc:///icon/ic_action_camera.png"
				onTriggered: {
					if (stackView.currentItem != scanner)
						stackView.push(scanner)
					else
						scanner.device = !scanner.device
				}
			},
			X.Action {
				icon: "qrc:///icon/ic_action_overflow.png"
				menu: X.Menu {
					actions: [
						X.Action {
							text: qsTr("작업 파일 열기")
							onTriggered: {
								fileDialog.files = X.Utility.files("*.txt")
								fileDialog.show()
							}
						},
						X.Action {
							text: qsTr("도서 원부 불러오기")
							onTriggered: Library.import_()
						}
					]
				}

			}
		]
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
				font.pixelSize: X.Theme.textSizeMicro
			}
		}
	}
	contentItem: StackView {
		id: stackView
		anchors.fill: parent
		initialItem: WorkList {}
		BarcodeScanner {
			id: scanner; width: parent.width; height: parent.height;
			visible: stackView.currentItem === this; device: 1
			onVisibleChanged: scanner.scanning = visible
			onBarcodesChanged: {
				for (var i=0; i<barcodes.length; ++i) {
					if (BookList.append(barcodes[i]))
							break
				}
			}
			Rectangle {
				color: ma.pressed ? X.Theme.highlight : X.Theme.background
				width: X.Theme.lineHeight; height: width
				opacity: 0.5
				anchors { top: parent.top; right: parent.right; margins: X.Theme.padding }
				X.Action {
					anchors.fill: parent
					icon: scanner.torch ? "qrc:///icon/ic_action_flash_off.png"
										: "qrc:///icon/ic_action_flash_on.png"
				}
				MouseArea {
					id: ma; anchors.fill: parent
					onClicked: scanner.torch = !scanner.torch
				}
			}
		}
	}

	focus: true
	Keys.onReleased: {
		if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
			if (stackView.depth > 1) {
				stackView.pop();
				event.accepted = true;
			} else { Qt.quit(); }
		}
	}
}
