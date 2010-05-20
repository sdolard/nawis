/*
 * n_file_suffix.cpp - file extention class
 * Copyright (C) 2008 Sebastien Dolard
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// Std
#include <iostream>

// Qt
#include <QVariant>

// App
#include "n_log.h"

#include "n_file_suffix.h"

NFileSuffix::NFileSuffix()
{
    m_category = NFileCategory_n::fcAll;
    m_shared   = true;
}

bool NFileSuffix::isValid() const
{
    return !m_name.isEmpty();
}

bool NFileSuffix::operator==(const NFileSuffix & fileSuffix) const
{
    return m_name == fileSuffix.m_name &&
            m_category == fileSuffix.m_category &&
            m_shared == fileSuffix.m_shared;
}

NFileSuffix NFileSuffixList::category(const QFileInfo & fi) const
{
    return value(fi.suffix().toLower());
}

void NFileSuffixList::dump() const
{
    NLOGD("NFileSuffixList", "Managed suffix:");
    QHashIterator<QString, NFileSuffix> i(*this);
    while (i.hasNext()) {
        i.next();
        NFileSuffix suffix = i.value();
        NLOGD("NFileSuffixList", QString("  key:%1").arg(i.key()));
        NLOGD("NFileSuffixList", QString("  name:%1").arg(suffix.name()));
        NLOGD("NFileSuffixList", QString("    shared:%1").arg(QVariant(suffix.shared()).toString()));
        NLOGD("NFileSuffixList", QString("    category:%1").arg(NFileCategory_n::fileCategoryName(suffix.category())));
    }
}

const QStringList NFileSuffixList::suffixList(NFileCategory_n::FileCategory fc) const
{
    QStringList suffixList;
    QHashIterator<QString, NFileSuffix> i(*this);
    while (i.hasNext()) {
        i.next();
        NFileSuffix suffix = i.value();
        if (suffix.category() != fc)
            continue;
        suffixList << suffix.name();
    }
    return suffixList;
}

const QStringList NFileSuffixList::toDirNameFilters() const
{
    QStringList filter;
    QHashIterator<QString, NFileSuffix> i(*this);
    while (i.hasNext()) {
        i.next();
        NFileSuffix suffix = i.value();
        filter << "*."  + suffix.name();
    }
    return filter;
}
