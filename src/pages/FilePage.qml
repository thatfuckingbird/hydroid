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
import Hydroid 1.0
import "../widgets"
import "../dialogs"

ColumnLayout {
    id: pageLayout
    property string title: {
        if(thumbGridModel.count == 1) {
            "1 file"
        } else {
            `${thumbGridModel.count} files`
        }
    }
    property bool needsMetadataRefresh: false

    onVisibleChanged: {
        if(needsMetadataRefresh && visible) {
            hydrusAPI.updateMetadata(thumbGridModel)
            needsMetadataRefresh = false
        }
    }

    SearchBar {
        id: searchBar
    }
    readonly property SearchBar searchBar: searchBar
    readonly property ThumbGridModel thumbGridModel: thumbGridModel
    readonly property TagListModel tagListModel: tagListModel
    readonly property Component viewerComponent: Qt.createComponent("../dialogs/Viewer.qml")

    Component {
        id: singleShot
        Timer {
            property var action: undefined
            running: true
            onTriggered: {
                if (action) action()
                this.destroy()
            }
        }
    }

    SplitView {
        id: mainSplit
        Layout.fillHeight: true
        Layout.fillWidth: true
        orientation: width > height ? Qt.Horizontal : Qt.Vertical
        GridView {
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            clip: true
            model: thumbGridModel
            cellWidth: 154
            cacheBuffer: 10000
            cellHeight: 129
            delegate: Rectangle {
                width: 154
                height: 129
                color: "transparent"
                Image {
                    id: thumbImage
                    visible: progress == 1.0
                    width: {
                        let widthRatio = Math.min(1.0, 150/sourceSize.width)
                        let imgWidth = sourceSize.width * widthRatio
                        let imgHeight = sourceSize.height * widthRatio
                        let heightRatio = Math.min(1.0, 125/imgHeight)
                        imgHeight = imgHeight * heightRatio
                        imgWidth = imgWidth * heightRatio
                        return imgWidth
                    }
                    height: {
                        let widthRatio = Math.min(1.0, 150/sourceSize.width)
                        let imgHeight = sourceSize.height * widthRatio
                        let heightRatio = Math.min(1.0, 125/imgHeight)
                        imgHeight = imgHeight* heightRatio
                        return imgHeight
                    }
                    source: `${hydroidSettings.getString("apiURL")}/get_files/thumbnail?file_id=${fileID}&Hydrus-Client-API-Access-Key=${hydroidSettings.getString("apiKey")}`
                    anchors.centerIn: parent
                    /*onStatusChanged: {
                        if(status === Image.Error) {
                            let oldSource = source
                            source = ''
                            singleShot.createObject(this, { action: function() { thumbImage.source = oldSource; console.log("reload") }, interval: 3000 })
                        }
                    }*/
                }
                BusyIndicator {
                    anchors.centerIn: parent
                    width: parent.width / 2.0
                    height: parent.height / 2.0
                    visible: !thumbImage.visible
                }
                Rectangle {
                    id: selection
                    color: "blue"
                    opacity: selected ? 0.3 : 0
                    anchors.fill: parent
                    radius: 0
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: selected = !selected
                    onDoubleClicked: createViewer({thumbGridModel: thumbGridModel,
                                                   filterMode: false,
                                                   fileIDs: thumbGridModel.allFileIDs(),
                                                   currentIndex: index})
                }
            }
        }
        Pane {
            id: sideBarPane
            SplitView.preferredWidth: window.width * 0.2
            SplitView.minimumWidth: 0
            ColumnLayout {
                width: parent.width
                height: parent.height
                RowLayout {
                    visible: thumbGridModel.filesLoading || thumbGridModel.metadataLoading
                    BusyIndicator {
                        id: loadingIndicator
                        Layout.preferredHeight: 0
                    }
                    Label {
                        id: loadingInfoLabel
                        Layout.fillWidth: true
                        text: {
                            if(thumbGridModel.filesLoading && thumbGridModel.metadataLoading)
                            {
                                `Waiting for file list and ${thumbGridModel.queuedMetadataUpdateCount} metadata updates…`
                            } else if(thumbGridModel.metadataLoading)
                            {
                                `Waiting for ${thumbGridModel.queuedMetadataUpdateCount} metadata updates…`
                            } else if(thumbGridModel.filesLoading)
                            {
                                "Waiting for file list…"
                            } else
                            {
                                ""
                            }
                        }
                    }
                    Component.onCompleted: loadingIndicator.Layout.preferredHeight = loadingInfoLabel.height
                }
                RowLayout {
                    visible: !thumbGridModel.filesLoading
                    Label {
                        Layout.fillWidth: true
                        text: `${thumbGridModel.count} files`
                        elide: Text.ElideRight
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Rectangle {
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        icon.source: "qrc:/assets/icons/selection.svg"
                        WideMenu {
                            id: selectionMenu
                            MenuItem {
                                text: "Clear selection"
                                onClicked: thumbGridModel.selectNone()
                            }
                            MenuItem {
                                text: "Select all"
                                onClicked: thumbGridModel.selectAll()
                            }
                            MenuItem {
                                text: "Invert selection"
                                onClicked: thumbGridModel.invertSelection()
                            }
                            MenuItem {
                                text: "Select archive"
                                onClicked: thumbGridModel.selectArchive()
                            }
                            MenuItem {
                                text: "Select inbox"
                                onClicked: thumbGridModel.selectInbox()
                            }
                            MenuItem {
                                text: "Select filtered"
                                onClicked: thumbGridModel.selectWithTags([hydroidSettings.getString("filterAcceptTag"), hydroidSettings.getString("filterRejectTag")])
                            }
                            MenuItem {
                                text: "Select not filtered"
                                onClicked: thumbGridModel.selectWithoutTags([hydroidSettings.getString("filterAcceptTag"), hydroidSettings.getString("filterRejectTag")])
                            }
                        }
                        onClicked: selectionMenu.popup()
                    }
                    ToolButton {
                        icon.source: "qrc:/assets/icons/filter.svg"
                        WideMenu {
                            id: filterMenu
                            MenuItem {
                                text: "Archive/delete filter"
                                onClicked: {
                                    let selection = thumbGridModel.selectedFileIDs();
                                    if(selection.length === 0) {
                                        selection = thumbGridModel.allFileIDs()
                                    }
                                    if(selection.length > 0) createViewer({thumbGridModel: thumbGridModel,
                                                                           filterMode: true,
                                                                           fileIDs: selection,
                                                                           currentIndex: 0})
                                }
                            }
                        }
                        onClicked: filterMenu.popup()
                    }
                    ToolButton {
                        id: hideButton
                        icon.source: "qrc:/assets/icons/arrow-collapse-right.svg"
                        onClicked: sideBarPane.SplitView.preferredWidth = 0
                    }
                }
                GroupBox {
                    id: singleFileInfo
                    title: "File metadata"
                    Layout.fillWidth: true
                    visible: thumbGridModel.singleSelectedItem != -1
                    function updateFileInfo(data) {
                        metadataNotLoadedLabel.visible = !data["valid"]
                        urlListView.visible = data["valid"]
                        urlTitleLabel.visible = data["valid"]
                        dimensionsLabel.visible = data["valid"]
                        fileMimeLabel.visible = data["valid"]
                        fileSizeLabel.visible = data["valid"]
                        urlModel.clear()
                        if(data["valid"]) {
                            fileMimeLabel.text = "File type: " + data["mime"]
                            fileSizeLabel.text = "Size: "+ data["formattedSize"]
                            dimensionsLabel.text = `Dimensions: ${data["width"]}×${data["height"]}`
                            for(let i = 0; i < data["urls"].length; ++i) {
                                urlModel.append({url: data["urls"][i]})
                            }
                        }
                    }

                    ColumnLayout {
                        anchors.fill: parent
                        Label {
                            id: fileMimeLabel
                        }
                        Label {
                            id: fileSizeLabel
                        }
                        Label {
                            id: dimensionsLabel
                        }
                        Label {
                            id: urlTitleLabel
                            text: "File URLs:"
                            visible: urlModel.count > 0
                        }
                        ListView {
                            id: urlListView
                            visible: model.count > 0
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            model: ListModel {
                                id: urlModel
                            }
                            delegate: Label {
                                text: `<a href="${url}">${url}</a>`
                                elide: Label.ElideRight
                                width: parent ? parent.width : 0
                                onLinkActivated: Qt.openUrlExternally(url)
                            }
                            Layout.preferredHeight: count * urlTitleLabel.implicitHeight
                        }
                        Label {
                            id: metadataNotLoadedLabel
                            text: "No metadata loaded yet for this file"
                        }
                    }
                }
                GroupBox {
                    title: `Tags (${tagListModel.count})`
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    ListView {
                        anchors.fill: parent

                        model: tagListModel
                        delegate: Label {
                            text: {
                                if(thumbGridModel.singleSelectedItem == -1) {
                                    return `${tagText} (${tagCount})`
                                }
                                return `${tagText}`
                            }
                        }
                        clip: true
                    }
                }
            }
        }
}
    ThumbGridModel {
        id: thumbGridModel
        onSingleSelectedItemChanged: {
            if(singleSelectedItem !== -1) {
                singleFileInfo.updateFileInfo(metadataCache.getItemData(singleSelectedItem))
            }
        }
    }

    TagListModel {
        id: tagListModel
    }

    Component.onCompleted: thumbGridModel.setTagListModel(tagListModel)

    function createViewer(props) {
        let viewer = viewerComponent.createObject(window, props)
        viewer.open()
    }

    function restoreSidebar() {
        sideBarPane.SplitView.preferredWidth = window.width * 0.2
    }
}
