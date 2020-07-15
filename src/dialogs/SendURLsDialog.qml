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

import Hydroid 1.0
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

Dialog {
    id: sendURLsDialog

    title: "Send URLs to Hydrus"
    modal: true
    onAboutToShow: inputText.text = ""
    anchors.centerIn: parent

    footer: DialogButtonBox {
        Button {
            id: sendButton

            text: "Send"
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            onClicked: hydrusAPI.sendURLs(inputText.text)
        }

        Button {
            id: cancelButton

            text: "Cancel"
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }

    }

    contentItem: StackLayout {
        ColumnLayout {
            id: inputLayout

            TextArea {
                id: inputText

                placeholderText: "Enter URLs here.\nURLs should be separated by whitespace or line breaks."
            }

        }

        Pane {
            id: sendPane
        }

    }

}
