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
import "../widgets"
import Hydroid 1.0

Popup {
    id: viewer
    width: window.width
    height: window.height
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    property bool filterMode: false
    property bool waitingForMetadata: false
    property var fileIDs: []
    property ThumbGridModel thumbGridModel: undefined
    property int currentIndex: -1
    property var idsToPrefetch: ({})

    Rectangle {
        id: floatingNotification
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 15
        color: "#AA000000"
        visible: false
        width: notificationText.implicitWidth * 1.25
        height: notificationText.implicitHeight * 1.25
        radius: 5
        z: 1
        Label {
            id: notificationText
            anchors.centerIn: parent
        }

        Timer {
            id: hideTimer
            interval: 500
            running: false
            repeat: false
            onTriggered: hideAnimation.start()
        }

        NumberAnimation on opacity {
            id: hideAnimation
            from: 1
            to: 0
            duration: 500
            onFinished: floatingNotification.visible = false
        }

        function showNotification(text) {
            hideTimer.stop()
            hideAnimation.stop()
            notificationText.text = text
            visible = true
            opacity = 1
            hideTimer.start()
        }
    }

    WideMenu {
        id: viewerMenu

        MenuItem {
            id: exitItem
            text: "Exit image viewer"
            onTriggered: {
                viewer.close()
             }
        }
    }

    StackLayout {
        id: stack
        anchors.fill: parent
    Flickable {
        id: imagePage
        width: parent.width
        height: parent.height
        contentHeight: height
        contentWidth: width
        clip: true
        //ScrollBar.vertical: ScrollBar{active: imagePage.contentHeight/imagePage.height > 1 ? true : false}
        //ScrollBar.horizontal: ScrollBar{active: imagePage.contentWidth/imagePage.width > 1 ? true : false}
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds

        function resetTransform() {
            resizeContent(parent.width, parent.height, Qt.point(0, 0))
            image.width = Math.min(imagePage.contentWidth, image.sourceSize.width)
            image.height = Math.min(imagePage.contentHeight, image.sourceSize.height)
            returnToBounds()
        }

        PinchArea {
            id: pinch
            width: Math.max(imagePage.contentWidth, imagePage.width)
            height: Math.max(imagePage.contentHeight, imagePage.height)

            property real initialWidth
            property real initialHeight

            onPinchStarted: {
                initialWidth = image.width
                initialHeight = image.height
            }

            onPinchUpdated: {
                image.width = initialWidth * pinch.scale
                image.height = initialHeight * pinch.scale
                imagePage.resizeContent(Math.max(image.width, imagePage.width),
                                    Math.max(image.height, imagePage.height),
                                    Qt.point(mouse.mouseX, mouse.mouseY))
            }

            onPinchFinished: {
                imagePage.returnToBounds()
            }

            Image {
                id: image
                width: Math.min(imagePage.contentWidth, sourceSize.width)
                height: Math.min(imagePage.contentHeight, sourceSize.height)
                fillMode: Image.PreserveAspectFit
                anchors.centerIn: parent
                onStatusChanged: {
                    if(image.status === Image.Ready) {
                        imagePage.resetTransform()
                    }
                }
            }
            MouseArea {
                id: mouse
                anchors.fill: parent
                hoverEnabled: true
                onDoubleClicked: {
                    image.width = Math.min(imagePage.contentWidth,
                                           image.sourceSize.width)
                    image.height = Math.min(imagePage.contentHeight,
                                            image.sourceSize.height)
                    imagePage.resizeContent(imagePage.width, imagePage.height,
                                        Qt.point(imagePage.width / 2,
                                                 imagePage.height / 2))
                    imagePage.returnToBounds()
                }
                onWheel: {
                    image.width = wheel.angleDelta.y / 120 * image.width * 0.1 + image.width
                    image.height = wheel.angleDelta.y / 120 * image.height * 0.1 + image.height
                    imagePage.resizeContent(Math.max(image.width, imagePage.width),
                                        Math.max(image.height, imagePage.height),
                                        Qt.point(mouse.mouseX, mouse.mouseY))
                    imagePage.returnToBounds()
                }
                onClicked: {
                    let right = mouse.x > (2/3)*width
                    let left = mouse.x < (1/3)*width
                    let top = mouse.y < (1/4)*height
                    let bottom = mouse.y > (3/4)*height
                    handleAreaClick(left, right, top, bottom)
                }
                onPressAndHold: viewerMenu.popup()
            }
        }
    }
    Rectangle {
        id: infoPage
        color: Material.background
        Column {
            anchors.centerIn: parent
            Label {
                id: infoPageText
            }
            Label {
                visible: viewer.filterMode
                text: "The filter will not change tags for this file."
            }
        }
        MouseArea {
            id: infoPageMouse
            anchors.fill: parent
            onClicked: {
                let right = mouse.x > (2/3)*width
                let left = mouse.x < (1/3)*width
                let top = mouse.y < (1/4)*height
                let bottom = mouse.y > (3/4)*height
                handleAreaClick(left, right, top, bottom)
            }
            onPressAndHold: viewerMenu.popup()
        }
    }
    Rectangle {
        id: errorPage
        color: Material.background
        Column {
            anchors.centerIn: parent
            Label {
                text: "Error."
            }
        }
        MouseArea {
            id: errorPageMouse
            anchors.fill: parent
            onPressAndHold: viewerMenu.popup()
        }
    }
    }

    function hasContent() {
        return currentIndex != -1 && fileIDs.length > 0
    }

    function updateDisplay() {
        if(!hasContent()) {
            showErrorPage()
            return
        }
        idsToPrefetch = {}
        let minMetadataIdx = Math.max(0, currentIndex-25)
        let maxMetadataIdx = Math.min(fileIDs.length-1, currentIndex+25)
        let minPrefetchIdx = Math.max(0, currentIndex-3)
        let maxPrefetchIdx = Math.min(fileIDs.length-1, currentIndex+3)//TODO: these should wrap around
        waitingForMetadata = true
        idsToPrefetch = {}
        for(let i = minPrefetchIdx; i <= maxPrefetchIdx; ++i) {
            idsToPrefetch[fileIDs[i]] = currentIndex === i
        }
        acquireMetadata(fileIDs.slice(minMetadataIdx, maxMetadataIdx+1))
    }

    function acquireMetadata(ids) {
        let idsMissingMetadata = []
        for(let i = 0; i < ids.length; ++i) {
            let metadata = metadataCache.getItemData(ids[i])
            if(metadata["valid"]) {
                prefetchIfNeeded(ids[i], metadata)
                if(ids[i] === fileIDs[currentIndex]) updateDisplayFromMetadata(metadata)
            } else {
                idsMissingMetadata.push(ids[i])
            }
        }
        hydrusAPI.requestMetadataForViewer(viewer, idsMissingMetadata)
    }

    function updateDisplayFromMetadata(metadata) {
        if(waitingForMetadata) {
            waitingForMetadata = false
            if(metadata["mime"].startsWith("image/")) {
                showImagePage()
            } else {
                showInfoPage(`This file can't be viewed in Hydroid yet (unsupported mime: ${metadata["mime"]}).`)
            }
        }
    }

    function handleNewMetadata(fileID, metadata) {
        if(fileID === fileIDs[currentIndex]) updateDisplayFromMetadata(metadata)
        prefetchIfNeeded(fileID, metadata)
    }

    function prefetchIfNeeded(fileID, metadata) {
        if(fileID in idsToPrefetch) {
            if(metadata["mime"].startsWith("image/")) fileCache.fetchImage(fileID, idsToPrefetch[fileID])
        }
    }

    function showImagePage() {
        image.source = "image://hyimg/"+Number(fileIDs[currentIndex]).toString()
        stack.currentIndex = 0
    }

    function showInfoPage(text) {
        infoPageText.text = text
        stack.currentIndex = 1
    }

    function showErrorPage() {
        stack.currentIndex = 2
    }

    function next() {
        if(!hasContent()) return
        if(currentIndex === fileIDs.length - 1) {
            if(filterMode) floatingNotification.showNotification("Filtering done (wrapped around to first image)")
            currentIndex = 0
        } else {
            currentIndex++
        }
        updateDisplay()
    }

    function previous() {
        if(!hasContent()) return
        if(currentIndex === 0) {
            currentIndex = fileIDs.length - 1
        } else {
            currentIndex--
        }
        updateDisplay()
    }

    function handleAreaClick(left, right, top, bottom) {
        if(left && !top && !bottom && !right) {
            if(filterMode && stack.currentIndex == 0) {
                let updateData = {}
                updateData[hydroidSettings.getString("localTagRepoName")] = {
                    "0": [hydroidSettings.getString("filterRejectTag")],
                    "1": [hydroidSettings.getString("filterAcceptTag")]
                }

                //TODO: this should be removed in favor of re-requesting (from Hydrus) file metadata after every change for the changed files
                //This is really just a temporary hack
                metadataCache.addTag(fileIDs[currentIndex], hydroidSettings.getString("filterRejectTag"))
                metadataCache.removeTag(fileIDs[currentIndex], hydroidSettings.getString("filterAcceptTag"))

                let metadata = metadataCache.getItemData(fileIDs[currentIndex])
                if(metadata["valid"]) {
                    hydrusAPI.updateTags(metadata["hash"], updateData)
                    floatingNotification.showNotification("Image rejected")
                } else {
                    floatingNotification.showNotification("Can't set tag: metadata not yet loaded")
                    acquireMetadata([fileIDs[currentIndex]])
                }
            } else if(filterMode) {
                floatingNotification.showNotification("No filter action taken as the file couldn't be displayed")
            }
            if(filterMode) {
                next()
            } else {
                previous()
            }
        } else
        if(right && !left && !bottom && !top) {
            if(filterMode && stack.currentIndex == 0) {
                let updateData = {}
                updateData[hydroidSettings.getString("localTagRepoName")] = {
                    "0": [hydroidSettings.getString("filterAcceptTag")],
                    "1": [hydroidSettings.getString("filterRejectTag")]
                }

                //TODO: this should be removed in favor of re-requesting (from Hydrus) file metadata after every change for the changed files
                //This is really just a temporary hack
                metadataCache.addTag(fileIDs[currentIndex], hydroidSettings.getString("filterAcceptTag"))
                metadataCache.removeTag(fileIDs[currentIndex], hydroidSettings.getString("filterRejectTag"))

                let metadata = metadataCache.getItemData(fileIDs[currentIndex])
                if(metadata["valid"]) {
                    hydrusAPI.updateTags(metadata["hash"], updateData)
                    floatingNotification.showNotification("Image accepted")
                } else {
                    floatingNotification.showNotification("Can't set tag: metadata not yet loaded")
                    acquireMetadata([fileIDs[currentIndex]])
                }
                floatingNotification.showNotification("Image accepted")
            } else if(filterMode) {
                floatingNotification.showNotification("No filter action taken as the file couldn't be displayed")
            }
            next()
        } else
        if(top && !left && !bottom && !right) {
            next()
        } else
        if(bottom && !top && !left && !right) {
            previous()
        }
    }

    Component.onCompleted: updateDisplay()

    onClosed: viewer.destroy()
}
