import QtQuick 2.11
import QtQuick.Window 2.11
import com.iskrembilen 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

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

        spacing: 10
        rows: Crossword.rows
        columns: Crossword.columns

        Repeater {
            model: Crossword.rows * Crossword.columns
            delegate: Rectangle {
                width: 100
                height: 100
                border.width: 2

                Text {
                    x: 10
                    y: 10
                    text: Crossword.hintAt(index)
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        currentHint.text = Crossword.hintTextAt(index)
                    }
                    onClicked: {
                        console.log(Crossword.correctAt(index))
                    }
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

        Repeater {
            model: Crossword.hintsDown()
            delegate: Text {
                text: modelData
            }
        }
    }
}
