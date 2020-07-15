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

#include "filecache.h"
#include "hydrusapi.h"

FileCache::FileCache(QObject* parent) :
    QObject(parent)
{
    connect(&HydrusAPI::hydrusAPI(), &HydrusAPI::fileReceived, this, &FileCache::handleFileReceived);
}

void FileCache::maintainCache()
{
    if(m_data.size() <= 12) return;

    std::sort(m_data.begin(), m_data.end(), [](const CacheEntry& a, const CacheEntry& b) {
        if(a.highPriorityRequest != b.highPriorityRequest) return a.highPriorityRequest > b.highPriorityRequest;
        return a.lastRequested > b.lastRequested;
    });

    for(int i = 12; i < m_data.size(); ++i)
    {
        HydrusAPI::hydrusAPI().cancelFileRequest(m_data[i].requestID);
    }

    m_data.resize(12);
}

void FileCache::handleFileReceived(int fileID, const QByteArray& data)
{
    QImage img = QImage::fromData(data);
    for(auto& entry: m_data)
    {
        if(entry.fileID == fileID) entry.image = img;
    }

    emit this->imageAvailable(fileID, img);

    maintainCache();
}

FileCache& FileCache::fileCache()
{
    static FileCache cache{nullptr};
    return cache;
}

void FileCache::fetchImage(int fileID, bool highPriority)
{
    for(auto& entry: m_data)
    {
        if(entry.fileID == fileID)
        {
            entry.lastRequested = m_reqCounter++;
            entry.highPriorityRequest = highPriority;
            if(!entry.image.isNull())
            {
                emit this->imageAvailable(fileID, entry.image);
            }
            else
            {
                HydrusAPI::hydrusAPI().updateFileRequestPriority(entry.requestID, highPriority);
            }
            return;
        }
    }

    m_data.push_back({.image = {},
                      .fileID = fileID,
                      .lastRequested = m_reqCounter++,
                      .requestID = HydrusAPI::hydrusAPI().requestFile(fileID, highPriority),
                      .highPriorityRequest = highPriority});

    maintainCache();
}

QImage FileCache::getImage(int fileID) const
{
    for(auto& entry: m_data)
    {
        if(entry.fileID == fileID) return entry.image;
    }
    return {};
}
