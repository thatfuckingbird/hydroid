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
import QtQuick 2.0
import QtQuick.Controls 2.14

TabButton {
    property var page: undefined

    function showMenu() {
        let menu = mainTabBar.tabPopupMenu.createObject(window, {tab: this, allowCloseOptions: mainTabBar.count > 3});
        menu.tab = this;
        menu.popup();
    }

    function close() {
        hydroidSettings.removeSavedPage(page.thumbGridModel.pageID);
        page.destroy();
        destroy();
    }

    function closeOthers() {
        for (let i = 0; i < mainTabBar.count - 2; ++i) {
            if (mainTabBar.itemAt(i) !== this)
                mainTabBar.itemAt(i).close();

        }
    }

    onPressAndHold: showMenu()
    text: page ? page.title : "Empty page"
}
