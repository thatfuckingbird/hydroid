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

import "../dialogs"
import "../widgets"
import Hydroid 1.0
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.14

RowLayout {
    id: searchBar

    function addTag(tag) {
        for (let i = 0; i < tagList.model.count; ++i) {
            if (tagList.model.get(i).tagText === tag)
                return ;

        }
        tagList.model.append({
            "tagText": tag
        });
        searchBar.saveToSettings();
    }

    function removeTag(tag) {
        for (let i = 0; i < tagList.model.count; ++i) {
            if (tagList.model.get(i).tagText === tag) {
                tagList.model.remove(i);
                break;
            }
        }
        searchBar.saveToSettings();
    }

    function saveToSettings() {
        if (thumbGridModel.pageID !== -1)
            hydroidSettings.savePageSearchTags(thumbGridModel.pageID, tagListModel.getTags());

    }

    function loadFromSettings() {
        if (thumbGridModel.pageID !== -1) {
            tagListModel.clear();
            let tags = hydroidSettings.getSavedPageTags(thumbGridModel.pageID);
            for (let i = 0; i < tags.length; i++) tagList.model.append({
                "tagText": tags[i]
            })
        }
    }

    function search(searchType) {
        let tags = [];
        let inbox = false;
        let archive = false;
        for (let i = 0; i < tagList.model.count; ++i) {
            let tagText = tagList.model.get(i).tagText;
            if (tagText === "system:archive")
                archive = true;
            else if (tagText === "system:inbox")
                inbox = true;
            else
                tags.push(tagText);
        }
        hydrusAPI.fileSearch(tags, inbox, archive, thumbGridModel, searchType);
    }

    Layout.fillWidth: true

    AddTagDialog {
        id: addTagDialog

        parent: ApplicationWindow.overlay
    }

    ListView {
        id: tagList

        property int calculatedHeight: listViewFm.boundingRect("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ").height * 1.1 + 6

        height: calculatedHeight
        Layout.alignment: Qt.AlignVCenter
        clip: true
        Layout.fillWidth: true
        Layout.leftMargin: 2
        orientation: Qt.Horizontal
        spacing: 6

        FontMetrics {
            id: listViewFm

            font.pointSize: searchButton.font.pointSize + 8
        }

        MouseArea {
            anchors.fill: parent
            onDoubleClicked: addTagDialog.open()
            z: -1
        }

        model: ListModel {
            id: tagListModel

            function getTags() {
                let result = [];
                for (let i = 0; i < count; ++i) {
                    result.push(get(i).tagText);
                }
                return result;
            }

        }

        delegate: Rectangle {
            property var colors: hydroidNativeUtils.tagToColors(label.text)

            z: 1
            radius: 2
            width: label.implicitWidth + 12
            height: tagList.calculatedHeight
            border.width: 0
            color: colors[0]

            Text {
                id: label

                font.pointSize: searchButton.font.pointSize + 8
                anchors.centerIn: parent
                text: tagText
                color: colors[1]
            }

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    parent.ListView.view.model.remove(index);
                    searchBar.saveToSettings();
                }
            }

        }

    }

    Button {
        icon.source: "qrc:/assets/icons/plus-box.svg"
        Layout.leftMargin: 6
        Layout.alignment: Qt.AlignLeft
        Layout.preferredWidth: implicitWidth * 2
        onClicked: addTagDialog.open()
    }

    Button {
        id: searchButton

        text: "Search"
        Layout.leftMargin: 6
        Layout.rightMargin: 2
        Layout.alignment: Qt.AlignRight
        Layout.preferredWidth: implicitWidth * 2
        onClicked: searchBar.search(hydrusAPI.FullSearch)
        onPressAndHold: searchMenu.popup()

        WideMenu {
            id: searchMenu

            MenuItem {
                text: "Search but do not load metadata"
                onClicked: searchBar.search(hydrusAPI.NoMetadataSearch)
            }

            MenuItem {
                text: "Refresh metadata for current files"
                onClicked: hydrusAPI.updateMetadata(thumbGridModel)
            }

        }

    }

}
