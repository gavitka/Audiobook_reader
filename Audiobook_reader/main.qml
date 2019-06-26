import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Universal 2.12
import Qt.labs.settings 1.0

import io.qt.examples.backend 1.0
import Theme 1.0
import "Components"

Window {
    id: window
    visible: true
    width: 640
    height: 480
    minimumWidth: 300
    minimumHeight: 480
    title: qsTr("Audiobook Reader")

    Settings {
        //property alias x: window.x
        //property alias y: window.y
        //property alias width: window.width
        //property alias height: window.height
    }

    RowLayout {
        anchors.fill:parent
        spacing: 0
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "red"
            ColumnLayout {
                spacing: 0
                anchors.fill: parent
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "slategray"

                    BookList {
                        id:booksPage
                        anchors.fill: parent
                        visible: true
                    }
                    Settings {
                        id:settingPage
                        anchors.fill: parent
                        visible: false
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    color: "khaki"
                    Button {
                        text:"settings"
                        onClicked: {
                            settingPage.visible = !settingPage.visible;
                            booksPage.visible = !booksPage.visible
                        }
                    }
                }
            }

        }
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: "blue"
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                Player2 {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
                }

                Playlist {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
}
