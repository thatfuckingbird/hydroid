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

import "../widgets"
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.14

Dialog {
    id: addTagDialog

    title: "Add tag to search query"
    modal: true
    standardButtons: Dialog.Cancel
    anchors.centerIn: parent
    onAboutToShow: {
        tagInput.text = "";
    }

    contentItem: ColumnLayout {
        GroupBox {
            Layout.fillWidth: true
            title: "Search for regular tag"

            RowLayout {
                anchors.fill: parent

                WideTextField {
                    id: tagInput

                    placeholderText: "tag"
                    onAccepted: {
                        searchBar.addTag(text);
                        addTagDialog.close();
                    }
                }

                Button {
                    text: "OK"
                    onClicked: {
                        searchBar.addTag(tagInput.text);
                        addTagDialog.close();
                    }
                }

            }

        }

        GroupBox {
            Layout.fillWidth: true
            title: "Search in"

            RowLayout {
                anchors.fill: parent

                Button {
                    text: "archive"
                    Layout.fillWidth: true
                    onClicked: {
                        searchBar.removeTag("system:inbox");
                        searchBar.addTag("system:archive");
                        addTagDialog.close();
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: "inbox"
                    onClicked: {
                        searchBar.removeTag("system:archive");
                        searchBar.addTag("system:inbox");
                        addTagDialog.close();
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: "both"
                    onClicked: {
                        searchBar.removeTag("system:inbox");
                        searchBar.removeTag("system:archive");
                        addTagDialog.close();
                    }
                }

            }

        }

    }

}
