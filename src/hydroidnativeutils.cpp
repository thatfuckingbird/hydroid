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

#include "hydroidnativeutils.h"
#include <QFile>
#include <QTextStream>

HydroidNativeUtils::HydroidNativeUtils(QObject* parent) :
    QObject(parent)
{
}

QString HydroidNativeUtils::readTextFile(const QString& path) const
{
    QFile f{path};
    if(f.open(QFile::ReadOnly))
    {
        QTextStream inStream(&f);
        return inStream.readAll();
    }
    return {};
}

const QStringList systemTags = {"system:inbox", "system:archive", "system:everywhere"};

QVariantList HydroidNativeUtils::tagToColors(const QString& tag) const
{
    if(systemTags.contains(tag))
    {
        return {QColor{144, 202, 249}, QColor{48, 48, 48}};
    }
    return {QColor{238, 238, 238}, QColor{48, 48, 48}};
}
