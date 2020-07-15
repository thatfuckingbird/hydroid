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

#include <QObject>
#include "taglistmodel.h"

TagListModel::TagListModel(QObject* parent) :
    QAbstractListModel(parent)
{
}

QHash<int, QByteArray> TagListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TagTextRole] = "tagText";
    roles[TagCountRole] = "tagCount";
    return roles;
}

Qt::ItemFlags TagListModel::flags(const QModelIndex&) const
{
    return Qt::ItemNeverHasChildren | Qt::ItemIsEnabled;
}

QVariant TagListModel::data(const QModelIndex& index, int role) const
{
    //Used to supply item data to views and delegates. Generally, models only need to supply data for Qt::DisplayRole and any application-specific user roles,
    //but it is also good practice to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.
    //See the Qt::ItemDataRole enum documentation for information about the types associated with each role.

    if(!index.internalPointer()) return {};

    if(role == TagTextRole)
    {
        return static_cast<TagListEntry*>(index.internalPointer())->tagText;
    }
    else if(role == TagCountRole)
    {
        return static_cast<TagListEntry*>(index.internalPointer())->count;
    }

    return {};
}

int TagListModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) return 0;
    return m_data.size();
}

bool TagListModel::setData(const QModelIndex&, const QVariant&, int)
{
    return false;
}

QModelIndex TagListModel::index(int row, int, const QModelIndex& parent) const
{
    if(parent.isValid()) return QModelIndex{};
    return this->createIndex(row, -1, const_cast<TagListEntry*>(m_data[row]));
}

QModelIndex TagListModel::parent(const QModelIndex&) const
{
    return QModelIndex{};
}

int TagListModel::count() const
{
    return m_data.size();
}

void TagListModel::beginUpdateTags()
{
    if(m_tagUpdateInProgress) qFatal("beginUpdateTags called while a tag update was already in progress");

    m_tagUpdateInProgress = true;
}

void TagListModel::addTags(int fileID, const QSet<QString>& tags)
{
    if(tags.isEmpty()) return;

    bool noTagChangeInProgress = !m_tagUpdateInProgress;
    if(noTagChangeInProgress) beginUpdateTags();

    if(m_fileIDsToTags.count(fileID))
    {
        addNewTagsInternal(tags - m_fileIDsToTags[fileID]);
        m_fileIDsToTags[fileID] += tags;
    }
    else
    {
        addNewTagsInternal(tags);
        m_fileIDsToTags[fileID] = tags;
    }
    if(noTagChangeInProgress) endUpdateTags();
}

void TagListModel::removeTags(int fileID)
{
    if(m_fileIDsToTags.count(fileID))
    {
        bool noTagChangeInProgress = !m_tagUpdateInProgress;
        if(noTagChangeInProgress) beginUpdateTags();

        for(const auto& tag: m_fileIDsToTags.value(fileID))
        {
            auto entry = findTagInSortedRange(tag, 0, m_data.size() - 1);
            if(entry != -1)
            {
                m_data[entry]->count--;
                if(m_data[entry]->count < 1)
                {
                    m_indicesConsideredForRemoval.insert(entry);
                }
            }
            else if(m_newTags.count(tag))
            {
                m_newTags[tag]--;
            }
        }
        m_fileIDsToTags.remove(fileID);
        if(noTagChangeInProgress) endUpdateTags();
    }
}

void TagListModel::endUpdateTags()
{
    const auto& cref_indicesConsideredForRemoval = m_indicesConsideredForRemoval; //To avoid detaching Qt container in the following loop.
    for(auto idx: cref_indicesConsideredForRemoval)
    {
        if(m_data[idx]->count < 1)
        {
            delete m_data[idx];
            m_data[idx] = nullptr;
        }
    }
    m_data.reserve(m_data.size() + m_newTags.size());
    const int oldSize = m_data.size();
    QHashIterator<QString, int> it{m_newTags};
    while(it.hasNext())
    {
        it.next();
        if(it.value() > 0) m_data.append(new TagListEntry{it.key(), it.value()});
    }

    m_indicesConsideredForRemoval.clear();
    m_newTags.clear();

    std::stable_sort(m_data.begin(), m_data.end(), [](const TagListEntry* a, const TagListEntry* b) {
        if(a && b) return a->tagText < b->tagText;
        if(a && !b) return true;
        return false;
    });

    int nullCnt = 0;
    int lastIdx = m_data.size() - 1;
    while(lastIdx >= 0 && !m_data[lastIdx])
    {
        nullCnt++;
        lastIdx--;
    }

    if(m_data.size() - nullCnt > oldSize)
    {
        this->beginInsertRows({}, oldSize, m_data.size() - nullCnt - 1);
        this->endInsertRows();
    }
    else if(m_data.size() - nullCnt < oldSize)
    {
        this->beginRemoveRows({}, m_data.size() - nullCnt, oldSize - 1);
        m_data.resize(m_data.size() - nullCnt);
        this->endRemoveRows();
    }

    if(oldSize != m_data.size()) emit this->countChanged(m_data.size());

    if(m_data.size()) emit this->dataChanged(this->createIndex(0, -1, &this->m_data.first()), this->createIndex(m_data.size() - 1, -1, &this->m_data.last()));

    m_tagUpdateInProgress = false;
}

void TagListModel::clear()
{
    if(m_data.size())
    {
        this->beginRemoveRows(QModelIndex{}, 0, m_data.size() - 1);
        const auto& cref_data = this->m_data; //To avoid detaching Qt container in the following loop.
        for(auto entry: cref_data) delete entry;
        this->m_data.clear();
        m_fileIDsToTags.clear();
        m_indicesConsideredForRemoval.clear();
        m_tagUpdateInProgress = false;
        emit this->countChanged(m_data.size());
        this->endRemoveRows();
    }
}

int TagListModel::findTagInSortedRange(const QString& text, int start, int end)
{
    while(start < end)
    {
        const int idx = start + (end - start) / 2;
        const int cmp = m_data[idx]->tagText.compare(text);
        if(cmp < 0)
        {
            start = idx + 1;
        }
        else if(cmp > 0)
        {
            end = idx - 1;
        }
        else
            return idx;
    }
    if(start == end)
    {
        if(m_data[start]->tagText == text) return start;
    }

    return -1;
}

bool TagListModel::tagUpdateInProgress() const
{
    return this->m_tagUpdateInProgress;
}

void TagListModel::addNewTagsInternal(const QSet<QString>& tags)
{
    for(const auto& tag: tags)
    {
        if(m_newTags.count(tag))
        {
            m_newTags[tag]++;
        }
        else
        {
            auto entry = findTagInSortedRange(tag, 0, m_data.size() - 1);
            if(entry != -1)
            {
                m_data[entry]->count++;
            }
            else
            {
                m_newTags[tag] = 1;
            }
        }
    }
}
