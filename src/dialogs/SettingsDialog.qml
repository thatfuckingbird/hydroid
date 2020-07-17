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
import Hydroid 1.0
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

Dialog {
    id: settingsDialog

    title: "Settings"
    modal: true
    standardButtons: Dialog.Close
    anchors.centerIn: parent
    onAboutToShow: {
        apiURL.text = hydroidSettings.getString("apiURL");
        apiKey.text = hydroidSettings.getString("apiKey");
        filterAcceptTag.text = hydroidSettings.getString("filterAcceptTag");
        filterRejectTag.text = hydroidSettings.getString("filterRejectTag");
        restoreLastSession.checked = hydroidSettings.getBoolean("restoreLastSessionOnStartup");
        metadataRequestSize.value = hydroidSettings.getInteger("metadataRequestSize");
        networkTimeout.value = hydroidSettings.getInteger("networkTimeout");
        localTagRepoName.text = hydroidSettings.getString("localTagRepoName");
        startFullScreen.checked = hydroidSettings.getBoolean("startFullScreen")
    }

    contentItem: ColumnLayout {
        GridLayout {
            columns: 2

            Label {
                text: "Hydrus API URL:"
            }

            WideTextField {
                id: apiURL

                onEditingFinished: hydroidSettings.setString("apiURL", text)
            }

            Label {
                text: "Hydrus API access key:"
            }

            WideTextField {
                id: apiKey

                onEditingFinished: hydroidSettings.setString("apiKey", text)
            }

            Label {
                text: "Filter accepted tag:"
            }

            WideTextField {
                id: filterAcceptTag

                onEditingFinished: hydroidSettings.setString("filterAcceptTag", text)
            }

            Label {
                text: "Filter rejected tag:"
            }

            WideTextField {
                id: filterRejectTag

                onEditingFinished: hydroidSettings.setString("filterRejectTag", text)
            }

            Label {
                text: "Name of the local tag repository"
            }

            WideTextField {
                id: localTagRepoName

                onEditingFinished: hydroidSettings.setString("localTagRepoName", text)
            }

            Label {
                text: "Fetch metadata for this many files in one request:"
            }

            SpinBox {
                id: metadataRequestSize

                from: 1
                to: 1000
                editable: true
                stepSize: 128
                onValueModified: hydroidSettings.setInteger("metadataRequestSize", value)

                validator: IntValidator {
                    locale: metadataRequestSize.locale.name
                    bottom: metadataRequestSize.from
                    top: metadataRequestSize.to
                }

            }

            Label {
                text: "Network timeout (0 to disable):"
            }

            SpinBox {
                id: networkTimeout

                from: 0
                to: 6.048e+08
                editable: true
                stepSize: 5000
                onValueModified: {
                    hydroidSettings.setInteger("networkTimeout", value);
                    hydrusAPI.setTimeout(value);
                }

                validator: IntValidator {
                    locale: networkTimeout.locale.name
                    bottom: networkTimeout.from
                    top: networkTimeout.to
                }

            }

        }

        CheckBox {
            id: restoreLastSession

            text: "Restore last session on startup"
            onCheckStateChanged: hydroidSettings.setBoolean("restoreLastSessionOnStartup", checked)
        }

        CheckBox {
            id: startFullScreen

            text: "Start fullscreen"
            onCheckStateChanged: hydroidSettings.setBoolean("startFullScreen", checked)
        }

    }

}
