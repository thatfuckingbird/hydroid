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

#include "hydrusapi.h"
#include "hydroidsettings.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QJsonArray>
#include <hydroidsettings.h>

HydrusAPI::HydrusAPI(QObject* parent) :
    QObject(parent)
{
    this->m_nam = new QNetworkAccessManager{this};
    this->m_nam->setTransferTimeout(HydroidSettings::hydroidSettings().getInteger("networkTimeout"));
    connect(this->m_nam, &QNetworkAccessManager::finished, this, &HydrusAPI::handleNetworkReplyFinished);
}

HydrusAPI& HydrusAPI::hydrusAPI()
{
    static HydrusAPI api{nullptr};
    return api;
}

void HydrusAPI::requestMetadataForViewer(QObject* viewer, const QVector<int>& fileIDs)
{
    QString array = "[";
    for(const int id: fileIDs)
    {
        array += QString::number(id);
        array += ",";
    }
    array.chop(1);
    array += "]";

    QMap<QString, QString> searchParams;
    searchParams["file_ids"] = QUrl::toPercentEncoding(array);

    connect(viewer, &QObject::destroyed, this, &HydrusAPI::viewerDestroyed, Qt::UniqueConnection);

    auto reply = this->get("/get_files/file_metadata", searchParams);
    m_metadataSearchJobsToViewers[reply] = viewer;
    m_viewersToMetadataSearchJobs.insert(viewer, reply);
}

void HydrusAPI::fileSearch(const QStringList& tags, bool inbox, bool archive, ThumbGridModel* targetModel, SearchType searchType)
{
    if(auto job = m_modelsToFileSearchJobs.value(targetModel, nullptr))
    {
        job->abort();
    }

    targetModel->setFilesLoading(true);

    QMap<QString, QString> searchParams;
    searchParams["system_inbox"] = inbox ? "true" : "false";
    searchParams["system_archive"] = archive ? "true" : "false";

    QJsonArray tagsArr = QJsonArray::fromStringList(tags);
    QJsonDocument tagsDoc;
    tagsDoc.setArray(tagsArr);

    searchParams["tags"] = QString::fromUtf8(QUrl::toPercentEncoding(tagsDoc.toJson()));

    connect(targetModel, &ThumbGridModel::destroyed, this, &HydrusAPI::modelDestroyed, Qt::UniqueConnection);

    auto reply = this->get("/get_files/search_files", searchParams);
    m_fileSearchJobsToModels[reply] = targetModel;
    m_modelsToFileSearchJobs[targetModel] = reply;
    if(searchType == NoMetadataSearch)
    {
        reply->setProperty("noMetadataSearch", true);
    }
}

int HydrusAPI::updateMetadata(ThumbGridModel* targetModel, const QVector<int>& fileIDs)
{
    if(targetModel->count() == 0) return 0;
    if(fileIDs.isEmpty())
    {
        return updateMetadata(targetModel, targetModel->allFileIDs());
    }

    targetModel->setMetadataLoading(true);

    int req_counter = 0;
    const int batchSize = HydroidSettings::hydroidSettings().getInteger("metadataRequestSize");
    for(int i = 0; i < fileIDs.size(); i += batchSize)
    {
        QString array = "[";
        for(int j = i; j < fileIDs.size() && j < i + batchSize; ++j)
        {
            array += QString::number(fileIDs[j]);
            array += ",";
        }
        array.chop(1);
        array += "]";

        QMap<QString, QString> searchParams;
        searchParams["file_ids"] = QUrl::toPercentEncoding(array);

        connect(targetModel, &ThumbGridModel::destroyed, this, &HydrusAPI::modelDestroyed, Qt::UniqueConnection);

        auto reply = this->get("/get_files/file_metadata", searchParams);
        m_metadataSearchJobsToModels[reply] = targetModel;
        m_modelsToMetadataSearchJobs.insert(targetModel, reply);
        ++req_counter;
    }
    targetModel->setQueuedMetadataUpdateCount(targetModel->queuedMetadataUpdateCount() + req_counter);
    return req_counter;
}

void HydrusAPI::sendURLs(const QString& text)
{
    const QStringList urlList = text.split(QRegularExpression{"\\s+"}, Qt::SkipEmptyParts);

    for(const auto& url: urlList)
    {
        QJsonDocument doc;
        QJsonObject obj;
        obj["url"] = url;
        doc.setObject(obj);
        this->post("/add_urls/add_url", doc); //TODO: check reply
    }
}

void HydrusAPI::updateFileRequestPriority(int, bool)
{
    //TODO...
}

int HydrusAPI::requestFile(int fileID, bool highPriority)
{
    int newReqID = this->m_fileRequestCounter++;

    auto existingRequestIDs = m_fileIDsToRequestIDs.values(fileID);
    m_fileIDsToRequestIDs.insert(fileID, newReqID);
    m_requestIDsToFileIDs.insert(newReqID, fileID);

    if(!existingRequestIDs.isEmpty())
    {
        for(auto id: existingRequestIDs)
        {
            this->updateFileRequestPriority(id, highPriority);
            auto job = m_requestIDsToFileJobs[id];
            if(!m_fileJobsToRequestIDs.contains(job, newReqID)) m_fileJobsToRequestIDs.insert(job, newReqID);
            m_requestIDsToFileJobs[newReqID] = job;
        }
    }
    else
    {
        QMap<QString, QString> params;
        params["file_id"] = QString::number(fileID);

        auto reply = this->get("/get_files/file", params, highPriority);

        m_requestIDsToFileJobs[newReqID] = reply;
        m_fileJobsToRequestIDs.insert(reply, newReqID);
    }

    return newReqID;
}

void HydrusAPI::cancelFileRequest(int requestID)
{
    removeFileRequest(requestID, true);
}

void HydrusAPI::setTimeout(int timeout)
{
    this->m_nam->setTransferTimeout(timeout);
}

void HydrusAPI::updateTags(const QString& hash, QVariant updateData)
{
    QJsonDocument doc;
    QJsonObject obj;
    obj["hash"] = hash;
    obj["service_names_to_actions_to_tags"] = QJsonObject::fromVariantMap(updateData.toMap());

    doc.setObject(obj);

    this->post("/add_tags/add_tags", doc); //TODO: check reply
}

QNetworkReply* HydrusAPI::get(const QString& endpoint, const QMap<QString, QString>& args, bool highPriority)
{
    auto apiURL = QUrl{HydroidSettings::hydroidSettings().getString("apiURL") + endpoint};
    const QString accessKey = HydroidSettings::hydroidSettings().getString("apiKey");

    QUrlQuery query{apiURL};
    for(auto it = args.begin(); it != args.end(); ++it) query.addQueryItem(it.key(), it.value());
    apiURL.setQuery(query);

    QNetworkRequest req{apiURL};
    if(highPriority) req.setPriority(QNetworkRequest::HighPriority);
    req.setRawHeader("Hydrus-Client-API-Access-Key", accessKey.toUtf8());

    return m_nam->get(req);
}

QNetworkReply* HydrusAPI::post(const QString& endpoint, const QJsonDocument& body, bool highPriority)
{
    auto apiURL = QUrl{HydroidSettings::hydroidSettings().getString("apiURL") + endpoint};
    const QString accessKey = HydroidSettings::hydroidSettings().getString("apiKey");

    QNetworkRequest req{apiURL};
    if(highPriority) req.setPriority(QNetworkRequest::HighPriority);
    req.setRawHeader("Hydrus-Client-API-Access-Key", accessKey.toUtf8());
    req.setRawHeader("Content-Type", "application/json");

    return m_nam->post(req, body.toJson());
}

void HydrusAPI::handleNetworkReplyFinished(QNetworkReply* reply)
{
    if(auto model = m_fileSearchJobsToModels.value(reply, nullptr))
    {
        m_fileSearchJobsToModels.remove(reply);
        m_modelsToFileSearchJobs.remove(model);

        if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
        {
            model->clearAndLoadData(QJsonDocument::fromJson(reply->readAll())["file_ids"].toArray());
            if(!reply->property("noMetadataSearch").toBool()) updateMetadata(model);
        }
        else
        {
            //TODO...
        }
    }
    else if(auto model = m_metadataSearchJobsToModels.value(reply, nullptr))
    {
        m_metadataSearchJobsToModels.remove(reply);
        if(m_modelsToMetadataSearchJobs.count(model))
        {
            m_modelsToMetadataSearchJobs.remove(model, reply);
        }

        if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
        {
            model->loadMetadata(QJsonDocument::fromJson(reply->readAll())["metadata"].toArray());
        }
        else
        {
            //TODO...
        }

        model->setQueuedMetadataUpdateCount(model->queuedMetadataUpdateCount() - 1);

        if(!m_modelsToMetadataSearchJobs.count(model)) model->setMetadataLoading(false);
    }
    else if(auto fileReqIDs = m_fileJobsToRequestIDs.values(reply); !fileReqIDs.isEmpty())
    {
        if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
        {
            const QByteArray data = reply->readAll();
            QSet<int> fileIDs; //This is actually just 1 ID (that can belong to multiple request IDs).
            for(const auto id: fileReqIDs)
            {
                fileIDs.insert(m_requestIDsToFileIDs.value(id));
                removeFileRequest(id, false);
            }
            for(const auto fId: fileIDs) emit this->fileReceived(fId, data);
        }
        else
        {
            for(const auto id: fileReqIDs)
            {
                removeFileRequest(id, false);
            }
            //TODO...
        }
    }
    else if(auto viewer = m_metadataSearchJobsToViewers.value(reply, nullptr))
    {
        m_viewersToMetadataSearchJobs.remove(viewer, reply);
        m_metadataSearchJobsToViewers.remove(reply);

        if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
        {
            auto metadata = QJsonDocument::fromJson(reply->readAll())["metadata"].toArray();
            for(const auto v: metadata)
            {
                QVariantMap obj = v.toObject().toVariantMap();
                obj["hasMetadata"] = true;
                obj["valid"] = true;
                QMetaObject::invokeMethod(viewer, "handleNewMetadata", Q_ARG(QVariant, obj["file_id"]), Q_ARG(QVariant, QVariant::fromValue(obj)));
            }
        }
        else
        {
            //TODO...
        }
    }

    reply->deleteLater();
}

void HydrusAPI::modelDestroyed(QObject* model)
{
    m_modelsToFileSearchJobs.remove(static_cast<ThumbGridModel*>(model));
    for(auto it = m_fileSearchJobsToModels.begin(); it != m_fileSearchJobsToModels.end();)
    {
        if(it.value() == model)
        {
            it = m_fileSearchJobsToModels.erase(it);
        }
        else
        {
            ++it;
        }
    }

    m_modelsToMetadataSearchJobs.remove(static_cast<ThumbGridModel*>(model));
    for(auto it = m_metadataSearchJobsToModels.begin(); it != m_metadataSearchJobsToModels.end();)
    {
        if(it.value() == model)
        {
            it = m_metadataSearchJobsToModels.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void HydrusAPI::viewerDestroyed(QObject* viewer)
{
    for(const auto v: m_viewersToMetadataSearchJobs.values(viewer))
    {
        m_metadataSearchJobsToViewers.remove(v);
    }
    m_viewersToMetadataSearchJobs.remove(viewer);
}

void HydrusAPI::removeFileRequest(int requestID, bool abort)
{
    auto request = m_requestIDsToFileJobs.value(requestID, nullptr);
    auto fileID = m_requestIDsToFileIDs.value(requestID, -1);
    m_requestIDsToFileJobs.remove(requestID);
    m_requestIDsToFileIDs.remove(requestID);
    if(fileID != -1) m_fileIDsToRequestIDs.remove(fileID, requestID);
    if(!request) return;
    auto allIDs = m_fileJobsToRequestIDs.values(request);
    if(allIDs.size() > 1)
    {
        m_fileJobsToRequestIDs.remove(request, requestID);
    }
    else
    {
        m_fileJobsToRequestIDs.remove(request);
        if(abort) request->abort();
    }
}
