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

#include <QSettings>

class ThumbGridModel;

class HydroidSettings : public QSettings
{
    Q_OBJECT

private:
    HydroidSettings(QObject* parent = nullptr);

public:
    static HydroidSettings& hydroidSettings();
    Q_INVOKABLE bool getBoolean(const QString& key) const;
    Q_INVOKABLE void setBoolean(const QString& key, bool value);
    Q_INVOKABLE QString getString(const QString& key) const;
    Q_INVOKABLE void setString(const QString& key, const QString& value);
    Q_INVOKABLE void removeSavedPage(int uid);
    Q_INVOKABLE void savePageModel(ThumbGridModel* model);
    Q_INVOKABLE void savePageSearchTags(int uid, const QStringList& tags);
    Q_INVOKABLE QVector<int> getSavedPageIDs();
    Q_INVOKABLE void restoreSavedPageModel(int uid, ThumbGridModel* targetModel);
    Q_INVOKABLE QStringList getSavedPageTags(int uid);
    Q_INVOKABLE void clearSavedSession();
    Q_INVOKABLE int getInteger(const QString& key) const;
    Q_INVOKABLE void setInteger(const QString& key, int value);

signals:
#ifdef Q_OS_WASM
    void settingsReady();
#endif
};
