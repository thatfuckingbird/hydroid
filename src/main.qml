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
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.14
import QtQuick.Window 2.14
import "dialogs"
import "widgets"

ApplicationWindow {
    id: window

    function createPage(pageID = -1) {
        let newPage = mainStack.filePage.createObject(mainStack);
        let newTab = mainTabBar.regularTab.createObject(mainTabBar, {
            "page": newPage
        });
        mainTabBar.insertItem(mainTabBar.count - 3, newTab);
        mainTabBar.setCurrentIndex(mainTabBar.count - 3);
        if (pageID !== -1) {
            hydroidSettings.restoreSavedPageModel(pageID, newPage.thumbGridModel);
            newPage.searchBar.loadFromSettings();
            newPage.needsMetadataRefresh = pageID !== -1;
        } else {
            hydroidSettings.savePageModel(newPage.thumbGridModel);
            newPage.searchBar.saveToSettings();
        }
    }

    visible: true
    title: "Hydroid"
    Material.theme: Material.Dark
    Material.accent: Material.Blue
    Component.onCompleted: {
        if(hydroidSettings.getBoolean("startFullScreen")) {
            showFullScreen()
        } else {
            showMaximized()
        }
        if(hydroidSettings.getBoolean("firstStart")) {
            firstStartDialog.open()
            hydroidSettings.setBoolean("firstStart", false)
        }
    }

    ColumnLayout {
        width: parent.width
        height: parent.height

        TabBar {
            id: mainTabBar

            property int lastIndexBeforeMenuButtonActivated: 0
            property Component regularTab: Qt.createComponent("qrc:/src/pages/FilePageTab.qml", Component.PreferSynchronous, mainTabBar)
            property Component tabPopupMenu: Qt.createComponent("qrc:/src/pages/TabPopupMenu.qml", Component.PreferSynchronous, mainTabBar)

            Layout.fillWidth: true
            onCurrentIndexChanged: {
                if (currentIndex > count - 3)
                    setCurrentIndex(Math.max(0, mainTabBar.count - 3));

            }
            Component.onCompleted: {
                if (hydroidSettings.getBoolean("restoreLastSessionOnStartup")) {
                    let savedPageIDs = hydroidSettings.getSavedPageIDs();
                    if (savedPageIDs.length > 0) {
                        for (let i = 0; i < savedPageIDs.length; ++i) {
                            window.createPage(savedPageIDs[i]);
                        }
                        hydrusAPI.updateMetadata(mainStack.children[mainTabBar.currentIndex].thumbGridModel);
                    } else {
                        window.createPage();
                    }
                } else {
                    hydroidSettings.clearSavedSession();
                    window.createPage();
                }
            }

            TabButton {
                id: newTabButton

                icon.source: "qrc:/assets/icons/plus-circle.svg"
                width: Math.max(implicitWidth, Math.min(1.5 * implicitWidth, window.width * 0.1))
                onReleased: {
                    window.createPage();
                }
            }

            TabButton {
                id: menuButton

                icon.source: "qrc:/assets/icons/menu.svg"
                width: Math.max(implicitWidth, Math.min(1.5 * implicitWidth, window.width * 0.1))
                onPressed: {
                    mainTabBar.lastIndexBeforeMenuButtonActivated = mainTabBar.currentIndex;
                }
                onClicked: {
                    mainTabBar.setCurrentIndex(mainTabBar.lastIndexBeforeMenuButtonActivated);
                    mainMenu.popup();
                }

                WideMenu {
                    id: mainMenu

                    MenuItem {
                        id: sendURLsItem

                        text: "Send URLs to Hydrus…"
                        onTriggered: sendURLsDialog.open()
                    }

                    MenuSeparator {
                    }

                    MenuItem {
                        text: "Settings…"
                        onTriggered: settingsDialog.open()
                    }

                    MenuSeparator {
                    }

                    MenuItem {
                        text: "Help"
                        onTriggered: helpDialog.open()
                    }

                    MenuItem {
                        text: "First start guide"
                        onTriggered: firstStartDialog.open()
                    }

                    MenuItem {
                        text: "About"
                        onTriggered: aboutDialog.open()
                    }

                    MenuSeparator {
                    }

                    MenuItem {
                        text: "Quit"
                        onTriggered: Qt.quit()
                    }

                }

            }

        }

        StackLayout {
            id: mainStack

            property Component filePage: Qt.createComponent("qrc:/src/pages/FilePage.qml", Component.PreferSynchronous, mainStack)

            currentIndex: Math.max(0, Math.min(mainTabBar.currentIndex, mainTabBar.count - 2))
        }

    }

    SettingsDialog {
        id: settingsDialog
    }

    SendURLsDialog {
        id: sendURLsDialog
    }

    HelpDialog {
        id: helpDialog
    }

    AboutDialog {
        id: aboutDialog
    }

    FirstStartDialog {
        id: firstStartDialog
    }
}
