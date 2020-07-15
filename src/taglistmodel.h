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
#include <QAbstractListModel>
#include <QSet>

struct TagListEntry
{
    QString tagText;
    int count = 0;
};

class TagListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum TagListRole
    {
        TagTextRole = Qt::UserRole + 1,
        TagCountRole
    };
    TagListModel(QObject* parent = nullptr);

    //QAbstractListModel functions
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual QModelIndex index(int row, int, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex&) const override;

    int count() const;

    void beginUpdateTags();
    void addTags(int fileID, const QSet<QString>& tags);
    void removeTags(int fileID);
    void endUpdateTags();
    void clear();
    int findTagInSortedRange(const QString& text, int start, int end);
    bool tagUpdateInProgress() const;

signals:
    void countChanged(int count);

private:
    void addNewTagsInternal(const QSet<QString>& tags);
    QVector<TagListEntry*> m_data;
    QSet<int> m_indicesConsideredForRemoval;
    QHash<QString, int> m_newTags;
    QHash<int, QSet<QString>> m_fileIDsToTags;
    bool m_tagUpdateInProgress = false;
};
