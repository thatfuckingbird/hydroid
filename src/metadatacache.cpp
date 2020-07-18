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

#include "metadatacache.h"
#include <QLocale>

MetadataCache &MetadataCache::metadataCache()
{
    static MetadataCache cache;
    return cache;
}

MetadataEntry &MetadataCache::entry(int fileID)
{
    return m_data[fileID];
}

void MetadataCache::beginDataUpdate()
{
    m_dataUpdateInProgress = true;
}

void MetadataCache::setData(int fileID, const QJsonObject &data)
{
    auto& item = m_data[fileID];
    item.size = data["size"].toInt();
    item.mime = data["mime"].toString();
    item.width = data["width"].toInt();
    item.height = data["height"].toInt();
    item.inbox = data["is_inbox"].toBool();
    item.local = data["is_local"].toBool();
    item.trashed = data["is_trashed"].toBool();
    item.hash = data["hash"].toString();
    item.valid = true;

    QJsonArray urls = data["known_urls"].toArray();
    for(int j = 0; j < urls.size(); ++j)
    {
        item.urls.append(urls[j].toString());
    }

    QJsonObject tags = data["service_names_to_statuses_to_tags"].toObject();
    for(const auto& tagRepo: tags.keys())
    {
        QJsonObject tagRepoTags = tags[tagRepo].toObject();
        if(tagRepoTags.contains("0"))
        {
            QJsonArray tagRepoCurrentTags = tagRepoTags["0"].toArray();
            for(int j = 0; j < tagRepoCurrentTags.size(); ++j)
            {
                item.tags.insert(tagRepoCurrentTags[j].toString());
            }
        }
    }

    if(m_dataUpdateInProgress) {
        m_updatedIDs.append(fileID);
    } else {
        emit this->dataUpdated({fileID});
    }
}

void MetadataCache::endDataUpdate()
{
    if(!m_updatedIDs.isEmpty()) emit this->dataUpdated(m_updatedIDs);
    m_updatedIDs.clear();
    m_dataUpdateInProgress = false;
}

MetadataCache::MetadataCache(QObject *parent) : QObject(parent)
{

}

QVariantMap MetadataCache::getItemData(int fileID) const
{
    QVariantMap m;
    if(const auto& item = m_data.value(fileID, {}); item.valid)
    {
        m["urls"] = item.urls;
        m["tags"] = QStringList{item.tags.begin(), item.tags.end()};
        m["size"] = item.size;
        m["mime"] = item.mime;
        m["width"] = item.width;
        m["height"] = item.height;
        m["inbox"] = item.inbox;
        m["trashed"] = item.trashed;
        m["local"] = item.local;
        m["hash"] = item.hash;
        m["formattedSize"] = QLocale{}.formattedDataSize(item.size);
        m["valid"] = true;
    }
    else
    {
        m["valid"] = false;
    }
    return m;
}
