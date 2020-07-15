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
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.14

Dialog {
    id: helpDialog

    title: "Help"
    modal: true
    standardButtons: Dialog.Close
    anchors.centerIn: parent
    height: parent.height * 0.9
    width: parent.width * 0.9

    contentItem: ScrollView {
        clip: true

        TextArea {
            readOnly: true
            text: hydroidNativeUtils.readTextFile(":/assets/help/help.html")
            textFormat: Text.RichText
            font.pointSize: 16
            wrapMode: Text.WordWrap
        }

    }

}
