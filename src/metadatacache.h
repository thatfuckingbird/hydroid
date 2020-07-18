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

#pragma once

#include <QObject>
#include <QSet>
#include <QHash>
#include <QVariantMap>
#include <QJsonObject>
#include <QJsonArray>

struct MetadataEntry
{
    MetadataEntry() {}
    int id = -1;
    bool inbox = false;
    bool trashed = false;
    bool local = false;
    int size = 0;
    int width = 0;
    int height = 0;
    QString mime;
    QStringList urls;
    QSet<QString> tags;
    QString hash;
    bool valid = false;
};

class MetadataCache : public QObject
{
    Q_OBJECT
public:
    static MetadataCache& metadataCache();
    MetadataEntry& entry(int fileID);
    const MetadataEntry& entry(int fileID) const;
    void beginDataUpdate();
    void setData(int fileID, const QJsonObject& data);
    void endDataUpdate();
    Q_INVOKABLE QVariantMap getItemData(int fileID) const;

signals:
    void dataUpdated(QVector<int> fileIDs);

private:
    explicit MetadataCache(QObject* parent = nullptr);
    QHash<int, MetadataEntry> m_data;
    bool m_dataUpdateInProgress = false;
    QVector<int> m_updatedIDs;
};
