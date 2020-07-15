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

#include <QQuickAsyncImageProvider>
#include <QQuickImageResponse>
#include <QImage>

class HydroidImageProvider : public QQuickAsyncImageProvider
{
public:
    HydroidImageProvider();
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override;
};

class HydroidImageResponse : public QQuickImageResponse
{
    Q_OBJECT

public:
    HydroidImageResponse(int fileID, const QSize& requestedSize);
    QQuickTextureFactory* textureFactory() const override;
    virtual ~HydroidImageResponse();

private slots:
    void handleImageAvailable(int fileID, QImage image);

private:
    int m_fileID = -1;
    QImage m_image;
};
