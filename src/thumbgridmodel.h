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
#include <QtQml>

struct ThumbGridItem
{
    ThumbGridItem() {}
    ThumbGridItem(int id, bool selected) :
        id(id), selected(selected) {}
    int id = -1;
    bool selected = false;
};

class TagListModel;

class ThumbGridModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool filesLoading READ filesLoading NOTIFY filesLoadingChanged)
    Q_PROPERTY(bool metadataLoading READ metadataLoading NOTIFY metadataLoadingChanged)
    Q_PROPERTY(int singleSelectedItem READ singleSelectedItem NOTIFY singleSelectedItemChanged)
    Q_PROPERTY(int pageID READ pageID)
    Q_PROPERTY(int queuedMetadataUpdateCount READ queuedMetadataUpdateCount WRITE setQueuedMetadataUpdateCount NOTIFY queuedMetadataUpdateCountChanged)

public:
    enum ThumbGridRole
    {
        FileIdRole = Qt::UserRole + 1,
        SelectedRole
    };
    ThumbGridModel(QObject* parent = nullptr);

    //QAbstractListModel functions
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual QModelIndex index(int row, int, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex&) const override;
    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void selectNone();
    Q_INVOKABLE void invertSelection();
    Q_INVOKABLE void selectInbox();
    Q_INVOKABLE void selectArchive();
    Q_INVOKABLE void selectWithTags(const QStringList& tags);
    Q_INVOKABLE void selectWithoutTags(const QStringList& tags);
    Q_INVOKABLE void setTagListModel(TagListModel* model);
    Q_INVOKABLE QVector<int> allFileIDs() const;
    Q_INVOKABLE QVector<int> selectedFileIDs() const;

    ThumbGridItem* getItemByFileID(int fileID) const;
    void clearAndLoadData(const QJsonArray& fileIDs);
    void loadMetadata(const QJsonArray& metadata);
    int count() const;
    int singleSelectedItem() const;
    bool filesLoading() const;
    bool metadataLoading() const;
    void setFilesLoading(bool filesLoading);
    void setMetadataLoading(bool metadataLoading);
    QByteArray serialize() const;
    void deserialize(int pageID, const QByteArray& data);
    int pageID() const;
    void setQueuedMetadataUpdateCount(int count);
    int queuedMetadataUpdateCount();

signals:
    void countChanged(int count);
    void filesLoadingChanged(bool filesLoading);
    void metadataLoadingChanged(bool metadataLoading);
    void singleSelectedItemChanged(int fileID);
    void queuedMetadataUpdateCountChanged(int count);

private:
    QList<ThumbGridItem> m_files;
    QMap<int, ThumbGridItem*> m_fileIDMap;
    QSet<ThumbGridItem*> m_selectedFiles;
    bool m_filesLoading = false;
    bool m_metadataLoading = false;
    TagListModel* m_tagListModel = nullptr;
    int m_pageID = -1;
    int m_singleSelectedItem = -1;
    int m_queuedMetadataUpdateCount = 0;
    bool m_selectionChangedFlag = true;
    static int m_pageIDCounter;
    void setItemSelected(ThumbGridItem& item, bool selected);
    void setItemSelected(ThumbGridItem* item, bool selected);
    void updateSingleSelectedItem(bool force = false);
    void fillTagListIfEmptySelection();
};
