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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "hydroidsettings.h"
#include "thumbgridmodel.h"
#include <QGuiApplication>
#include <QVariantMap>
#include <QNetworkRequest>
#include <QTimer>

#if defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
constexpr auto settingsFormat = QSettings::NativeFormat;
#else
constexpr auto settingsFormat = QSettings::IniFormat;
#endif

const QVariantMap defaultSettings = {
  {"apiURL", "http://127.0.0.1:45869"},
  {"apiKey", ""},
  {"filterAcceptTag", "hydroid:defaultFilterAccepted"},
  {"filterRejectTag", "hydroid:defaultFilterRejected"},
  {"restoreLastSessionOnStartup", true},
  {"metadataRequestSize", 256},
  {"networkTimeout", QNetworkRequest::DefaultTransferTimeoutConstant},
  {"localTagRepoName", "my tags"}};

HydroidSettings::HydroidSettings(QObject* parent) :
    QSettings{settingsFormat, QSettings::UserScope, "Hydroid", "Hydroid", parent}
{
#ifdef Q_OS_WASM
    std::function<void(void)> *testSettingsReady = new std::function<void(void)>();
    *testSettingsReady = [=](){
        if (this->status() == QSettings::NoError) {
            delete testSettingsReady;
            emit this->settingsReady();
        } else {
            QTimer::singleShot(10, *testSettingsReady);
        }
    };
    (*testSettingsReady)();
#endif
}

HydroidSettings& HydroidSettings::hydroidSettings()
{
    static HydroidSettings settings{nullptr};
    return settings;
}

bool HydroidSettings::getBoolean(const QString& key) const
{
    if(!this->contains(key))
    {
        if(defaultSettings.contains(key)) return defaultSettings.value(key).toBool();
        qFatal("Invalid config key: %s", key.toStdString().data());
    }
    return this->value(key).toBool();
}

void HydroidSettings::setBoolean(const QString& key, bool value)
{
    this->setValue(key, value);
    sync();
}

QString HydroidSettings::getString(const QString& key) const
{
    if(!this->contains(key))
    {
        if(defaultSettings.contains(key)) return defaultSettings.value(key).toString();
        qFatal("Invalid config key: %s", key.toStdString().data());
    }
    return this->value(key).toString();
}

void HydroidSettings::setString(const QString& key, const QString& value)
{
    this->setValue(key, value);
    sync();
}

void HydroidSettings::removeSavedPage(int uid)
{
    beginGroup("pages");
    beginGroup(QString::number(uid));
    remove("files");
    remove("searchBarTags");
    endGroup();
    endGroup();
    sync();
}

void HydroidSettings::savePageModel(ThumbGridModel* model)
{
    const auto serializedValue = model->serialize();
    beginGroup("pages");
    beginGroup(QString::number(model->pageID()));
    setValue("files", serializedValue);
    endGroup();
    endGroup();
    sync();
}

void HydroidSettings::savePageSearchTags(int uid, const QStringList& tags)
{
    beginGroup("pages");
    beginGroup(QString::number(uid));
    setValue("searchBarTags", tags);
    endGroup();
    endGroup();
    sync();
}

void HydroidSettings::restoreSavedPageModel(int uid, ThumbGridModel* targetModel)
{
    beginGroup("pages");
    beginGroup(QString::number(uid));
    const auto serializedData = value("files").toByteArray();
    endGroup();
    endGroup();
    targetModel->deserialize(uid, serializedData);
}

QStringList HydroidSettings::getSavedPageTags(int uid)
{
    beginGroup("pages");
    beginGroup(QString::number(uid));
    QStringList val = value("searchBarTags").toStringList();
    endGroup();
    endGroup();
    return val;
}

void HydroidSettings::clearSavedSession()
{
    remove("pages");
    sync();
}

int HydroidSettings::getInteger(const QString& key) const
{
    if(!this->contains(key))
    {
        if(defaultSettings.contains(key)) return defaultSettings.value(key).toInt();
        qFatal("Invalid config key: %s", key.toStdString().data());
    }
    return this->value(key).toInt();
}

void HydroidSettings::setInteger(const QString& key, int value)
{
    this->setValue(key, value);
    sync();
}

QVector<int> HydroidSettings::getSavedPageIDs()
{
    beginGroup("pages");
    QVector<int> res;
    for(const auto& group: childGroups())
    {
        res.append(group.toInt());
    }
    endGroup();
    return res;
}
