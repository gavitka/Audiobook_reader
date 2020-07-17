import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Qt.labs.folderlistmodel 2.12
import io.qt.examples.backend 1.0
import io.qt.examples.audiobookmodel 1.0
import Theme 1.0

Rectangle {
    color: Theme.background_color_dark2

    ListView {
        id: playlistView
        anchors.fill: parent
        spacing: 0
        clip: true

        model: AudioBookModel {
           id: audioBookModel
           audioBook: BackEnd.audioBook
        }

        currentIndex: audioBookModel.index

        Component {
            id: playlistDelegate
            Item {
                id: wrapper
                height: 50
                width: playlistView.width

                Button {
                    anchors.topMargin: 0
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    anchors.bottomMargin: 0
                    anchors.fill: parent

                    state: (down | wrapper.ListView.isCurrentItem) ?    "pressed" :
                            hovered ?                                   "hover" :
                            playlistView.currentIndex > index ?         "done" :
                                                                        "default"

                    contentItem:
                        Item {
                        anchors.fill:parent
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                id: fileLabel
                                leftPadding: 10
                                font.pixelSize: 16
                                text: model.text
                                color: Theme.accent2
                            }
                        }

                    background:
                        Rectangle {
                        id:backgroundRect
                        color: Theme.background_color_dark2
                            Rectangle {
                                width:parent.width
                                height:1
                                anchors.bottom:parent.bottom
                                color:Theme.dark_separator
                            }
                        } // Rectangle:backgroundRect
                    states: [
                        State {
                            name : "hover"
                            PropertyChanges {
                                target: backgroundRect
                                color: Theme.dark_accent
                            }
                            PropertyChanges {
                                target: fileLabel
                                color: Theme.accent2
                            }
                        },
                        State {
                            name : "pressed"
                            PropertyChanges {
                                target: backgroundRect
                                color: Theme.dark_accent
                            }
                            PropertyChanges {
                                target: fileLabel
                                color: Theme.accent2
                            }
                        },
                        State {
                            name : "default"
                            PropertyChanges {
                                target: backgroundRect
                                color: Theme.background_color_dark2
                            }
                            PropertyChanges {
                                target: fileLabel
                                color: Theme.accent2
                            }
                        },
                        State {
                            name : "done"
                            PropertyChanges {
                                target: backgroundRect
                                color: Theme.background_color_dark2
                            }
                            PropertyChanges {
                                target: fileLabel
                                color: Theme.dark_text
                            }
                        }
                    ] // states
                    transitions: Transition {
                        ColorAnimation {
                            duration: 50
                        }
                    }
                    onClicked:
                        audioBookModel.index = index
                } //Button
            } //Rectangle
        } //Component

        delegate: playlistDelegate
        ScrollBar.vertical: ScrollBar { }
    }
}
