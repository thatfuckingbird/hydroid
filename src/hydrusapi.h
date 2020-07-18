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

#include <map>
#include <QObject>
#include "thumbgridmodel.h"

class QNetworkAccessManager;
class QNetworkReply;
class HydroidSettings;

class HydrusAPI : public QObject
{
    Q_OBJECT

public:
    static HydrusAPI& hydrusAPI();
    Q_INVOKABLE void requestMetadataForViewer(QObject* viewer, const QVector<int>& fileIDs);

    enum SearchType
    {
        FullSearch,
        NoMetadataSearch,
    };
    enum TagOperation
    {
        Add,
        Delete
    };
    Q_ENUM(SearchType)
    Q_ENUM(TagOperation)

public slots:
    void fileSearch(const QStringList& tags, bool inbox, bool archive, ThumbGridModel* targetModel, SearchType searchType);
    int updateMetadata(ThumbGridModel* targetModel);
    int updateMetadataForIDs(const QVector<int>& fileIDs = {});
    void sendURLs(const QString& text);
    int requestFile(int fileID, bool highPriority);
    void updateFileRequestPriority(int fileID, bool highPriority);
    void cancelFileRequest(int requestID);
    void setTimeout(int timeout);
    void updateTags(const QString& hash, QVariant updateData);

signals:
    void fileReceived(int fileID, const QByteArray& data);

private slots:
    void handleNetworkReplyFinished(QNetworkReply* reply);
    void modelDestroyed(QObject* model);
    void viewerDestroyed(QObject* viewer);

private:
    explicit HydrusAPI(QObject* parent = nullptr);
    void removeFileRequest(int requestID, bool abort);
    QNetworkAccessManager* m_nam = nullptr;
    QNetworkReply* get(const QString& endpoint, const QMap<QString, QString>& args, bool highPriority = false);
    QNetworkReply* post(const QString& endpoint, const QJsonDocument& body, bool highPriority = false);
    QMap<ThumbGridModel*, QNetworkReply*> m_modelsToFileSearchJobs;
    QMap<QNetworkReply*, ThumbGridModel*> m_fileSearchJobsToModels;
    QMap<QNetworkReply*, QObject*> m_metadataSearchJobsToViewers;
    QMultiMap<QObject*, QNetworkReply*> m_viewersToMetadataSearchJobs;
    int m_fileRequestCounter = 0;
    QMultiMap<int, int> m_fileIDsToRequestIDs;
    QMap<int, int> m_requestIDsToFileIDs;
    QMap<int, QNetworkReply*> m_requestIDsToFileJobs;
    QMultiMap<QNetworkReply*, int> m_fileJobsToRequestIDs;
    QSet<QNetworkReply*> m_metadataUpdateJobs;
};
