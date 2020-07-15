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

#include <QImage>
#include <QObject>

struct CacheEntry
{
    QImage image;
    int fileID = -1;
    int lastRequested = -1;
    int requestID = -1;
    bool highPriorityRequest = false;
};

class FileCache : public QObject
{
    Q_OBJECT

private:
    FileCache(QObject* parent = 0);
    QVector<CacheEntry> m_data;
    int m_reqCounter = 0;
    void maintainCache();

private slots:
    void handleFileReceived(int fileID, const QByteArray& data);

public:
    static FileCache& fileCache();
    Q_INVOKABLE void fetchImage(int fileID, bool highPriority = false);
    Q_INVOKABLE QImage getImage(int fileID) const;

signals:
    void imageAvailable(int fileID, QImage image);
};
