import QtQuick 2.3
import QtQuick.Window 2.2
import com.iskrembilen 1.0

TabletWindow {
    visible: true
    width: 1404
    height: 1872

    title: qsTr("Hello World")
    flags: Qt.Dialog

    Rectangle {
        anchors.fill: parent
    }

    Text {
        id: currentHint
        anchors {
            bottom: parent.bottom
            bottomMargin: 20
            horizontalCenter: parent.horizontalCenter
        }
        font.pointSize: 20
        width: parent.width / 3
    }

    Grid {
        id: mainGrid
        anchors.centerIn: parent

        spacing: 20
        rows: Crossword.rows
        columns: Crossword.columns

        Repeater {
            model: Crossword.rows * Crossword.columns
            delegate: DrawableCell {
                width: 100
                height: 100

                enabled: !correctText.visible

                Rectangle {
                    anchors.fill: parent
                    border.width: 2
                    color: correctText.visible ? "black" : "transparent"
                }

                Text {
                    x: 10
                    y: 10
                    text: Crossword.hintAt(index)
                    color: correctText.visible ? "white" : "black"
                }


                Text {
                    id: correctText
                    anchors.centerIn: parent
                    visible: parent.recognized === text
                    text: Crossword.correctAt(index)
                    color: "white"
                }
            }
        }
    }

    Column {
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 10
            right: mainGrid.left
            rightMargin: 10
        }

        Text {
            text: "Across"
        }

        Repeater {
            model: Crossword.hintsAcross()
            delegate: Text {
                text: modelData
            }
        }
    }

    Column {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 10
        }

        Text {
            text: "Down"
        }

        Repeater {
            model: Crossword.hintsDown()
            delegate: Text {
                text: modelData
            }
        }
    }
}
