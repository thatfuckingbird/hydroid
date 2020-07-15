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

#include "hydroidsettings.h"
#include "thumbgridmodel.h"
#include <QGuiApplication>
#include <QVariantMap>
#include <QNetworkRequest>

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
    QSettings(parent)
{
}

HydroidSettings& HydroidSettings::hydroidSettings()
{
    static HydroidSettings settings{nullptr};
    return settings;
}

void HydroidSettings::waitForReady() const
{
#ifdef Q_OS_WASM
    //QSettings is async on wasm, have to wait until it becomes available.
    while(this->status() != QSettings::NoError)
    {
        QGuiApplication::processEvents();
    }
#endif
}

void HydroidSettings::sync()
{
#ifdef Q_OS_WASM
    QSettings::sync();
#endif
}

bool HydroidSettings::getBoolean(const QString& key) const
{
    waitForReady();
    if(!this->contains(key))
    {
        if(defaultSettings.contains(key)) return defaultSettings.value(key).toBool();
        qFatal("Invalid config key: %s", key.toStdString().data());
    }
    return this->value(key).toBool();
}

void HydroidSettings::setBoolean(const QString& key, bool value)
{
    waitForReady();
    this->setValue(key, value);
    sync();
}

QString HydroidSettings::getString(const QString& key) const
{
    waitForReady();
    if(!this->contains(key))
    {
        if(defaultSettings.contains(key)) return defaultSettings.value(key).toString();
        qFatal("Invalid config key: %s", key.toStdString().data());
    }
    return this->value(key).toString();
}

void HydroidSettings::setString(const QString& key, const QString& value)
{
    waitForReady();
    this->setValue(key, value);
    sync();
}

void HydroidSettings::removeSavedPage(int uid)
{
    waitForReady();
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
    waitForReady();
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
    waitForReady();
    beginGroup("pages");
    beginGroup(QString::number(uid));
    setValue("searchBarTags", tags);
    endGroup();
    endGroup();
    sync();
}

void HydroidSettings::restoreSavedPageModel(int uid, ThumbGridModel* targetModel)
{
    waitForReady();
    beginGroup("pages");
    beginGroup(QString::number(uid));
    const auto serializedData = value("files").toByteArray();
    endGroup();
    endGroup();
    targetModel->deserialize(uid, serializedData);
}

QStringList HydroidSettings::getSavedPageTags(int uid)
{
    waitForReady();
    beginGroup("pages");
    beginGroup(QString::number(uid));
    QStringList val = value("searchBarTags").toStringList();
    endGroup();
    endGroup();
    return val;
}

void HydroidSettings::clearSavedSession()
{
    waitForReady();
    remove("pages");
    sync();
}

int HydroidSettings::getInteger(const QString& key) const
{
    waitForReady();
    if(!this->contains(key))
    {
        if(defaultSettings.contains(key)) return defaultSettings.value(key).toInt();
        qFatal("Invalid config key: %s", key.toStdString().data());
    }
    return this->value(key).toInt();
}

void HydroidSettings::setInteger(const QString& key, int value)
{
    waitForReady();
    this->setValue(key, value);
    sync();
}

QVector<int> HydroidSettings::getSavedPageIDs()
{
    waitForReady();
    beginGroup("pages");
    QVector<int> res;
    for(const auto& group: childGroups())
    {
        res.append(group.toInt());
    }
    endGroup();
    return res;
}
