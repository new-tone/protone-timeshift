import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 400
    height: 300
    title: "Timeshift Player"

    property var ts: Qt.formatDateTime(new Date("2025-03-14T14:15:00"), "yyyy-MM-ddTHH:mm:ss")

    Button {
        id: playButton
        anchors.centerIn: parent
        text: "Play"
        onClicked: {
            timeshiftPlayer.playAt(ts)
        }
    }

}
