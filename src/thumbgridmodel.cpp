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

#include "thumbgridmodel.h"
#include "hydroidsettings.h"
#include "taglistmodel.h"
#include "hydrusapi.h"
#include "metadatacache.h"

int ThumbGridModel::m_pageIDCounter = 0;

ThumbGridModel::ThumbGridModel(QObject* parent) :
    QAbstractListModel(parent), m_pageID(m_pageIDCounter++)
{
    connect(&MetadataCache::metadataCache(), &MetadataCache::dataUpdated, this, &ThumbGridModel::handleMetadataUpdate);
}

QHash<int, QByteArray> ThumbGridModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FileIdRole] = "fileID";
    roles[SelectedRole] = "selected";
    return roles;
}

Qt::ItemFlags ThumbGridModel::flags(const QModelIndex&) const
{
    return Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant ThumbGridModel::data(const QModelIndex& index, int role) const
{
    //Used to supply item data to views and delegates. Generally, models only need to supply data for Qt::DisplayRole and any application-specific user roles,
    //but it is also good practice to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.
    //See the Qt::ItemDataRole enum documentation for information about the types associated with each role.

    if(!index.internalPointer()) return {};

    if(role == FileIdRole)
    {
        return static_cast<ThumbGridItem*>(index.internalPointer())->id;
    }
    else if(role == SelectedRole)
    {
        return static_cast<ThumbGridItem*>(index.internalPointer())->selected;
    }

    return {};
}

int ThumbGridModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) return 0;
    return m_files.size();
}

bool ThumbGridModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    //The dataChanged() signal should be emitted if the data was successfully set.
    if(role == SelectedRole)
    {
        if(!index.internalPointer()) return false;

        setItemSelected(static_cast<ThumbGridItem*>(index.internalPointer()), value.toBool());
        emit this->dataChanged(index, index, {SelectedRole});
        updateSingleSelectedItem();
        return true;
    }
    return false;
}

QModelIndex ThumbGridModel::index(int row, int, const QModelIndex& parent) const
{
    if(parent.isValid()) return QModelIndex{};
    return this->createIndex(row, -1, const_cast<ThumbGridItem*>((&m_files[row])));
}

QModelIndex ThumbGridModel::parent(const QModelIndex&) const
{
    return QModelIndex{};
}

bool ThumbGridModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if(parent.isValid()) return false;
    this->beginInsertRows(parent, row, row + count - 1);
    for(int i = 0; i < count; ++i)
    {
        this->m_files.insert(row, {});
    }
    this->endInsertRows();
    emit this->countChanged(this->count());
    return true;
}

bool ThumbGridModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if(parent.isValid()) return false;
    this->beginRemoveRows(parent, row, row + count - 1);
    for(int i = 0; i < count; ++i)
    {
        this->m_fileIDMap.remove(this->m_files[row].id);
        this->m_files.removeAt(row);
    }
    this->endRemoveRows();
    emit this->countChanged(this->count());
    return true;
}

void ThumbGridModel::selectAll()
{
    if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
    for(auto& f: m_files) setItemSelected(f, true);
    if(this->m_tagListModel) this->m_tagListModel->endUpdateTags();
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()));
    updateSingleSelectedItem();
    fillTagListIfEmptySelection();
}

void ThumbGridModel::selectNone()
{
    m_selectedFiles.clear();
    for(auto& f: m_files) f.selected = false;
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()), {SelectedRole});
    updateSingleSelectedItem();
    fillTagListIfEmptySelection();
}

void ThumbGridModel::invertSelection()
{
    if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
    for(auto& f: m_files) setItemSelected(f, !f.selected);
    if(this->m_tagListModel) this->m_tagListModel->endUpdateTags();
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()), {SelectedRole});
    updateSingleSelectedItem();
    fillTagListIfEmptySelection();
}

void ThumbGridModel::selectInbox()
{
    if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
    for(auto& f: m_files) setItemSelected(f, MetadataCache::metadataCache().entry(f.id).inbox);
    if(this->m_tagListModel) this->m_tagListModel->endUpdateTags();
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()), {SelectedRole});
    updateSingleSelectedItem();
    fillTagListIfEmptySelection();
}

void ThumbGridModel::selectArchive()
{
    if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
    for(auto& f: m_files) setItemSelected(f, !MetadataCache::metadataCache().entry(f.id).inbox);
    if(this->m_tagListModel) this->m_tagListModel->endUpdateTags();
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()), {SelectedRole});
    updateSingleSelectedItem();
    fillTagListIfEmptySelection();
}

void ThumbGridModel::selectWithTags(const QStringList& tags)
{
    if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
    const int tagsSize = tags.size();
    for(auto& f: m_files)
    {
        int tagsFound = 0;
        for(const auto& tag: tags)
        {
            if(MetadataCache::metadataCache().entry(f.id).tags.contains(tag)) tagsFound++;
        }
        setItemSelected(f, tagsFound == tagsSize);
    }
    if(this->m_tagListModel) this->m_tagListModel->endUpdateTags();
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()), {SelectedRole});
    updateSingleSelectedItem();
    fillTagListIfEmptySelection();
}

void ThumbGridModel::selectWithoutTags(const QStringList& tags)
{
    if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
    for(auto& f: m_files)
    {
        bool ok = true;
        for(const auto& tag: tags)
        {
            if(MetadataCache::metadataCache().entry(f.id).tags.contains(tag))
            {
                ok = false;
                break;
            }
        }
        setItemSelected(f, ok);
    }
    if(this->m_tagListModel) this->m_tagListModel->endUpdateTags();
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()), {SelectedRole});
    updateSingleSelectedItem();
    fillTagListIfEmptySelection();
}

void ThumbGridModel::setTagListModel(TagListModel* model)
{
    this->m_tagListModel = model;
    if(!this->m_tagListModel) return;

    this->m_tagListModel->clear();
    if(!m_selectedFiles.isEmpty())
    {
        if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
        for(const auto& f: m_selectedFiles)
        {
            this->m_tagListModel->addTags(f->id, MetadataCache::metadataCache().entry(f->id).tags);
        }
        this->m_tagListModel->endUpdateTags();
    }
    else
        fillTagListIfEmptySelection();
}

ThumbGridItem* ThumbGridModel::getItemByFileID(int fileID) const
{
    return m_fileIDMap.value(fileID, nullptr);
}

void ThumbGridModel::clearAndLoadData(const QJsonArray& fileIDs)
{
    if(m_files.size()) {
        this->beginRemoveRows({}, 0, m_files.size() - 1);
        m_fileIDMap.clear();
        m_files.clear();
        m_selectedFiles.clear();
        this->endRemoveRows();
    }
    if(m_tagListModel) m_tagListModel->clear();
    m_selectionChangedFlag = true;
    this->insertRows(0, fileIDs.size(), {});
    for(int i = 0; i < fileIDs.size(); ++i)
    {
        this->m_files[i].id = fileIDs[i].toInt();
        this->m_files[i].selected = false;
        this->m_fileIDMap[fileIDs[i].toInt()] = &m_files[i];
    }
    if(fileIDs.size())
    {
        emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(fileIDs.size() - 1, -1, &this->m_files.last()));
    }
    updateSingleSelectedItem();
    this->setFilesLoading(false);
    this->m_waitingForMetadataUpdate.clear();
    this->setQueuedMetadataUpdateCount(0);
    HydroidSettings::hydroidSettings().savePageModel(this);
}

int ThumbGridModel::count() const
{
    return m_files.size();
}

int ThumbGridModel::singleSelectedItem() const
{
    return m_singleSelectedItem;
}

bool ThumbGridModel::filesLoading() const
{
    return m_filesLoading;
}

bool ThumbGridModel::metadataLoading() const
{
    return m_metadataLoading;
}

void ThumbGridModel::setFilesLoading(bool filesLoading)
{
    this->m_filesLoading = filesLoading;
    emit this->filesLoadingChanged(filesLoading);
}

void ThumbGridModel::setMetadataLoading(bool metadataLoading)
{
    this->m_metadataLoading = metadataLoading;
    emit this->metadataLoadingChanged(metadataLoading);
}

QByteArray ThumbGridModel::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);
    stream << m_files.size();
    for(const auto& f: m_files)
    {
        stream << f.id << f.selected;
    }
    return data;
}

void ThumbGridModel::deserialize(int pageID, const QByteArray& data)
{
    this->m_pageID = pageID;
    if(m_pageIDCounter <= m_pageID) m_pageIDCounter = m_pageID + 1;
    QDataStream stream(data);
    int size;
    stream >> size;
    if(size > 0)
    {
        this->beginInsertRows(QModelIndex{}, 0, size - 1);
        for(int i = 0; i < size; ++i)
        {
            int id;
            bool selected;
            stream >> id;
            stream >> selected;
            m_files.append({id, selected});
            m_fileIDMap[id] = &m_files.back();
        }
        this->endInsertRows();
        if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()));
        emit this->countChanged(this->count());
        updateSingleSelectedItem();
    }
}

int ThumbGridModel::pageID() const
{
    return m_pageID;
}

QVector<int> ThumbGridModel::allFileIDs() const
{
    QVector<int> res;
    res.reserve(m_files.size());
    for(const auto& f: m_files)
    {
        res.append(f.id);
    }
    return res;
}

QVector<int> ThumbGridModel::selectedFileIDs() const
{
    QVector<int> res;
    res.reserve((m_selectedFiles.size()));
    for(const auto f: m_selectedFiles)
    {
        res.append(f->id);
    }
    return res;
}

void ThumbGridModel::setQueuedMetadataUpdateCount(int count)
{
    m_queuedMetadataUpdateCount = count;
    emit this->queuedMetadataUpdateCountChanged(count);
    this->setMetadataLoading(m_queuedMetadataUpdateCount > 0);
}

int ThumbGridModel::queuedMetadataUpdateCount()
{
    return m_queuedMetadataUpdateCount;
}

void ThumbGridModel::setWaitingForMetadataUpdate(const QVector<int> &fileIDs)
{
    this->m_waitingForMetadataUpdate = QSet<int>{fileIDs.cbegin(), fileIDs.cend()};
    this->setQueuedMetadataUpdateCount(this->m_waitingForMetadataUpdate.size());
}

void ThumbGridModel::setItemSelected(ThumbGridItem& item, bool selected)
{
    if(!item.selected && selected)
    {
        if(this->m_tagListModel && this->m_selectedFiles.isEmpty()) this->m_tagListModel->clear();
        m_selectedFiles.insert(&item);
        item.selected = true;
        m_selectionChangedFlag = true;
        if(m_tagListModel) m_tagListModel->addTags(item.id, MetadataCache::metadataCache().entry(item.id).tags);
    }
    else if(item.selected && !selected)
    {
        m_selectedFiles.remove(&item);
        item.selected = false;
        if(m_tagListModel) m_tagListModel->removeTags(item.id);
    }
    fillTagListIfEmptySelection();
}

void ThumbGridModel::setItemSelected(ThumbGridItem* item, bool selected)
{
    ThumbGridItem& itemRef = *item;
    setItemSelected(itemRef, selected);
}

void ThumbGridModel::updateSingleSelectedItem(bool force)
{
    int oldSingleSelected = m_singleSelectedItem;
    if(m_selectedFiles.size() == 1)
    {
        m_singleSelectedItem = (*m_selectedFiles.begin())->id;
    }
    else
    {
        m_singleSelectedItem = -1;
    }
    if(force || oldSingleSelected != m_singleSelectedItem) emit this->singleSelectedItemChanged(m_singleSelectedItem);
}

void ThumbGridModel::fillTagListIfEmptySelection()
{
    if(m_selectionChangedFlag && this->m_tagListModel)
    {
        if(m_selectedFiles.isEmpty())
        {
            m_selectionChangedFlag = false;
            this->m_tagListModel->clear();
            this->m_tagListModel->beginUpdateTags();
            for(const auto& f: m_files)
            {
                this->m_tagListModel->addTags(f.id, MetadataCache::metadataCache().entry(f.id).tags);
            }
            this->m_tagListModel->endUpdateTags();
        }
    }
}

void ThumbGridModel::handleMetadataUpdate(const QVector<int> &fileIDs)
{
    if(fileIDs.empty()) return;
    if(this->m_tagListModel) this->m_tagListModel->beginUpdateTags();
    int queuedUpdateFulfilledCount = 0;
    for(auto id: fileIDs) {
        queuedUpdateFulfilledCount += m_waitingForMetadataUpdate.remove(id);
        if(auto fileEntry = m_fileIDMap.value(id, nullptr))
        {
            if(this->m_tagListModel && (fileEntry->selected || m_selectedFiles.isEmpty())) {
                this->m_tagListModel->removeTags(fileEntry->id);
                this->m_tagListModel->addTags(fileEntry->id, MetadataCache::metadataCache().entry(fileEntry->id).tags);
            }
        } else
        {
            //TODO...
        }
    }
    if(this->m_tagListModel) this->m_tagListModel->endUpdateTags();
    this->setQueuedMetadataUpdateCount(this->queuedMetadataUpdateCount() - queuedUpdateFulfilledCount);
    //TODO: optimize so not the entire range is updated but only the actual changes
    if(this->m_files.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_files.first()), this->createIndex(m_files.size() - 1, -1, &this->m_files.last()));
    updateSingleSelectedItem(true);
}
