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

#include "hydroidimageprovider.h"
#include "filecache.h"

HydroidImageProvider::HydroidImageProvider()
{
}

QQuickImageResponse* HydroidImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
    return new HydroidImageResponse(id.toInt(), requestedSize);
}

HydroidImageResponse::HydroidImageResponse(int fileID, const QSize&) :
    m_fileID(fileID)
{
    if(m_image = FileCache::fileCache().getImage(fileID); !m_image.isNull())
    {
        emit this->finished();
    }
    else
    {
        connect(&FileCache::fileCache(), &FileCache::imageAvailable, this, &HydroidImageResponse::handleImageAvailable);
        FileCache::fileCache().fetchImage(fileID, true);
    }
}

QQuickTextureFactory* HydroidImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

HydroidImageResponse::~HydroidImageResponse()
{
    disconnect(&FileCache::fileCache(), &FileCache::imageAvailable, this, &HydroidImageResponse::handleImageAvailable);
}

void HydroidImageResponse::handleImageAvailable(int fileID, QImage image)
{
    if(fileID == this->m_fileID)
    {
        this->m_image = image;
        emit this->finished();
        disconnect(&FileCache::fileCache(), &FileCache::imageAvailable, this, &HydroidImageResponse::handleImageAvailable);
    }
}
