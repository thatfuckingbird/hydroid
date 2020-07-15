/*
Hydroid
Copyright (C) 2020  Hydroid developers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls.Material 2.12

Dialog {
    id: aboutDialog
    title: "About"
    modal: true
    standardButtons: Dialog.Close
    anchors.centerIn: parent
    height: parent.height * 0.9
    contentItem: ColumnLayout {
        spacing: 0
        TabBar {
            id: aboutTabBar
            Layout.fillWidth: true

            TabButton {
                text: "About Hydroid"
                onClicked: aboutLayout.currentIndex = 0
            }

            TabButton {
                text: "License"
                onClicked: aboutLayout.currentIndex = 1
            }
        }
        StackLayout {
            id: aboutLayout

            ColumnLayout {
                Layout.fillWidth: true
                Image {
                    source: "qrc:/assets/icons/icon.png"
                    Layout.alignment: Qt.AlignHCenter
                }
                Label {
                    font.pointSize: 32
                    text: "Hydroid"
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
                Label {
                    font.pointSize: 16
                    text: "A Hydrus client."
                    Layout.alignment: Qt.AlignHCenter
                }
                Label {
                    font.pointSize: 16
                    text: `version ${HYDROID_VERSION_MAJOR}.${HYDROID_VERSION_MINOR}.${HYDROID_VERSION_PATCH}${HYDROID_VERSION_LABEL.length > 0 ? "-"+HYDROID_VERSION_LABEL : ""}, \"${HYDROID_EDITION_TEXT}\" edition`
                    Layout.alignment: Qt.AlignHCenter
                }
                Label {
                    font.pointSize: 16
                    text: "Released under the GNU Affero General Public License (v3.0+)."
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            ScrollView {
                clip: true
                TextArea {
                    readOnly: true
                    text: hydroidNativeUtils.readTextFile(":/assets/help/agpl-3.0-standalone.html")
                    textFormat: Text.RichText
                    font.pointSize: 16
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
