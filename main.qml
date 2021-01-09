import QtQuick 2.6
import QtQuick.Window 2.2
import com.iskrembilen 1.0

TabletWindow {
    visible: true
    width: 1404
    height: 1872

    title: "reCrossable"
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

    Rectangle {
        anchors.fill: mainGrid
        anchors.margins: -(border.width  + 2)
        color: "transparent"
        border.width: 5
        border.color: "black"
    }

    Grid {
        id: mainGrid
        anchors {
            right: parent.right
            margins: 20
            bottom: parent.bottom
            left: downHints.right
            top: acrossHints.bottom
        }

        spacing: 1
        rows: Crossword.rows
        columns: Crossword.columns

        property int cellSize: Math.max(Math.floor(Math.min(width / Crossword.columns, height / Crossword.rows)), 80);

        Repeater {
            model: Crossword.rows * Crossword.columns

            delegate: DrawableCell {
                width: mainGrid.cellSize
//                width: 1404 - downHints.width
                height: mainGrid.cellSize
                onWidthChanged: console.log("cell widtH:" + width)

                enabled: Crossword.isOpen(index) && !correctText.visible

                Rectangle {
                    anchors.fill: parent
                    border.width: 1
                    color: parent.enabled ? "transparent" : "black"
                }

                Text {
                    x: 5
                    y: 5
                    text: Crossword.hintAt(index)
                    color: parent.enabled ? "gray" : "white"
                    font.pixelSize: 20
                    font.bold: true
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

    Rectangle {
        anchors.fill: downHints
        anchors.margins: -(border.width  + 2)
        color: "transparent"
        border.width: 5
        border.color: "black"
    }

    Column {
        id: downHints
        anchors {
            top: acrossHints.bottom
            topMargin: 20
            left: parent.left
            leftMargin: 10
        }

        Text {
            text: "Down"
            font.bold: true
        }

        Repeater {
            model: Crossword.hintsDown()
            delegate: Text {
                text: modelData
                font.pixelSize: 20
                width: 200
                wrapMode: Text.WordWrap
            }
        }
    }



    Rectangle {
        anchors.fill: acrossHints
        anchors.margins: -(border.width  + 2)
        color: "transparent"
        border.width: 5
        border.color: "black"
    }

    Grid {
        id: acrossHints
        anchors {
            right: parent.right
            left: parent.left
            top: parent.top
            margins: 10
        }

        Text {
            text: "Across"
            font.bold: true
        }

        Repeater {
            model: Crossword.hintsAcross()
            delegate: Text {
                text: modelData
                font.pixelSize: 20
            }
        }
    }
}
