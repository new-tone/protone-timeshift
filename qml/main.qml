import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import QtQuick.Effects

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 500
    title: "Radio 929 Timeshift Player"
    color: "#212121"
    
    // Default timestamp to start with
    property var currentTimestamp: new Date("2025-04-11T15:30:00")
    
    // Metadata update
    Connections {
        target: timeshiftPlayer
        function onCurrentMetadataChanged() {
            metadataText.text = timeshiftPlayer.currentMetadata
        }
        
        function onCurrentDateTimeChanged() {
            var date = timeshiftPlayer.currentDateTime
            timeLabel.text = Qt.formatDateTime(date, "yyyy-MM-dd HH:mm:ss")
            
            // Update the date picker to reflect current playback time
            if (datePickerLoader.item) {
                datePickerLoader.item.selectedDate = date
            }
            if (timePickerLoader.item) {
                timePickerLoader.item.hour = date.getHours()
                timePickerLoader.item.minute = date.getMinutes()
            }
        }
        
        function onIsPlayingChanged() {
            playPauseButton.icon.source = timeshiftPlayer.isPlaying ?
                        "image://icon/pause" : "image://icon/play_arrow"
        }
        
        function onPlaybackError(errorMessage) {
            errorDialog.text = errorMessage
            errorDialog.open()
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        
        // Header with current metadata
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#333333"
            radius: 5
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                
                Image {
                    id: radioIcon
                    source: "image://icon/radio"
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    fillMode: Image.PreserveAspectFit
                    source.color: "red"
                }
                
                Label {
                    id: metadataText
                    Layout.fillWidth: true
                    text: "Radio 929"
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    elide: Text.ElideRight
                }
            }
        }
        
        // Time display
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "#424242"
            radius: 5
            
            Label {
                id: timeLabel
                anchors.centerIn: parent
                text: Qt.formatDateTime(currentTimestamp, "yyyy-MM-dd HH:mm:ss")
                font.pixelSize: 16
                color: "white"
            }
        }
        
        // Playback controls
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 10
            
            RoundButton {
                icon.source: "image://icon/arrow_back"
                icon.width: 60
                icon.height: 60
                implicitWidth: 90
                implicitHeight: 90
                ToolTip.text: "Skip backward"
                ToolTip.visible: hovered
                
                onClicked: {
                    timeshiftPlayer.skipBackward(5)
                }
            }
            
            RoundButton {
                id: playPauseButton
                icon.source: "image://icon/play_arrow"
                icon.width: 80
                icon.height: 80
                implicitWidth: 120
                implicitHeight: 120
                
                onClicked: {
                    if (timeshiftPlayer.isPlaying) {
                        timeshiftPlayer.pause()
                    } else if (timeshiftPlayer.currentDateTime.getTime() > 0) {
                        timeshiftPlayer.resume()
                    } else {
                        timeshiftPlayer.playAt(currentTimestamp)
                    }
                }
            }
            
            RoundButton {
                icon.source: "image://icon/arrow_forward"
                icon.width: 60
                icon.height: 60
                implicitWidth: 90
                implicitHeight: 90
                ToolTip.text: "Skip forward"
                ToolTip.visible: hovered
                
                onClicked: {
                    timeshiftPlayer.skipForward(5)
                }
            }
        }
        
        // Date & Time picker
        Rectangle {
            Layout.fillWidth: true
            height: 120
            color: "#424242"
            radius: 5
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                
                Label {
                    text: "Jump to specific time:"
                    color: "white"
                    font.pixelSize: 14
                }
                
                RowLayout {
                    spacing: 20
                    
                    // Date selector
                    Loader {
                        id: datePickerLoader
                        objectName: "datePickerLoader"
                        sourceComponent: datePickerComponent
                        //Layout.preferredWidth: 300
                        
                        onLoaded: {
                            item.selectedDate = currentTimestamp
                        }
                    }
                    
                    // Time selector
                    Loader {
                        id: timePickerLoader
                        sourceComponent: timePickerComponent
                        
                        onLoaded: {
                            item.hour = currentTimestamp.getHours()
                            item.minute = currentTimestamp.getMinutes()
                        }
                    }
                    
                    Button {
                        text: "Go"
                        onClicked: {
                            var date = new Date(datePickerLoader.item.selectedDate)
                            date.setHours(timePickerLoader.item.hour)
                            date.setMinutes(timePickerLoader.item.minute)
                            date.setSeconds(0)
                            currentTimestamp = date
                            timeshiftPlayer.playAt(date)
                        }
                    }
                }
            }
        }
    }
    
    // Error dialog
    Dialog {
        id: errorDialog
        title: "Playback Error"
        standardButtons: Dialog.Ok
        property alias text: errorText.text
        
        Label {
            id: errorText
            width: parent.width
        }
        
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
    }
    // Date Picker Dialog Component
    Component {
        id: dateDialogComponent
        
        Dialog {
            id: dateDialog
            title: "Select Date"
            standardButtons: Dialog.Ok | Dialog.Cancel
            modal: true
            visible: false
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: parent.width * 0.8
            height: 250
            
            // Date that will be passed back
            property date selectedDate: new Date()
            property var callback: null
            
            // Initialize dialog with selected date
            Component.onCompleted: {
                yearSpin.value = selectedDate.getFullYear()
                monthSpin.value = selectedDate.getMonth() + 1
                daySpin.value = selectedDate.getDate()
            }
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                
                RowLayout {
                    spacing: 10
                    
                    // Year spinner
                    ColumnLayout {
                        Label {
                            text: "Year"
                            font.pixelSize: 12
                        }
                        SpinBox {
                            id: yearSpin
                            from: 2020
                            to: 2030
                            value: new Date().getFullYear()
                        }
                    }
                    
                    // Month spinner
                    ColumnLayout {
                        Label {
                            text: "Month"
                            font.pixelSize: 12
                        }
                        SpinBox {
                            id: monthSpin
                            from: 1
                            to: 12
                            value: new Date().getMonth() + 1
                        }
                    }
                    
                    // Day spinner
                    ColumnLayout {
                        Label {
                            text: "Day"
                            font.pixelSize: 12
                        }
                        SpinBox {
                            id: daySpin
                            from: 1
                            to: 31
                            value: new Date().getDate()
                        }
                    }
                }
            }
            
            onAccepted: {
                var newDate = new Date(yearSpin.value, monthSpin.value - 1, daySpin.value)
                if (callback) {
                    callback(newDate)
                }
            }
        }
    }
    
    // Simple DatePicker component
    Component {
        id: datePickerComponent
        
        Rectangle {
            id: datePicker
            
            property date selectedDate: new Date()
            
            height: 50
            width: 200

            color: "#333333"
            radius: 5
            
            function openDateDialog() {
                var dialog = dateDialogComponent.createObject(window, {
                                                                  "selectedDate": selectedDate,
                                                                  "callback": function(newDate) {
                                                                      selectedDate = newDate
                                                                  }
                                                              })
                dialog.open()
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                
                Label {
                    text: Qt.formatDate(datePicker.selectedDate, "yyyy-MM-dd")
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                    Layout.fillWidth: true
                }
                
                Button {
                    text: "..."
                    implicitWidth: height
                    onClicked: datePicker.openDateDialog()
                }
            }
        }
    }
    
    // TimePicker Component
    Component {
        id: timePickerComponent
        
        RowLayout {
            id: timePicker
            width: 300
            height: 50
            
            property int hour: 0
            property int minute: 0
            
            spacing: 5
            
            SpinBox {
                id: hourSpin
                from: 0
                to: 23
                value: hour
                Layout.preferredWidth: 70
                onValueChanged: hour = value
                Layout.fillWidth: true
            }
            
            Label {
                text: ":"
                color: "white"
                font.pixelSize: 20
                font.bold: true
                Layout.minimumWidth: 20
            }
            
            SpinBox {
                id: minuteSpin
                from: 0
                to: 59
                value: minute
                Layout.preferredWidth: 70
                onValueChanged: minute = value
                Layout.fillWidth: true
            }
        }
    }
}
