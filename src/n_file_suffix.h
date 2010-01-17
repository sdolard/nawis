/*
 * n_file_suffix.h - file extention class
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

#ifndef N_FILE_SUFFIX_H
#define N_FILE_SUFFIX_H

// Qt
#include <QString>
#include <QHash>
#include <QFileInfo>
#include <QStringList>

// App
#include "n_file_category.h"

class NFileSuffix
{
public:
	NFileSuffix();

	const QString & name() const { return m_name; }
	NFileCategory_n::FileCategory category() const { return m_category; }
	bool shared() const { return m_shared; } //default is true

	void setName(const QString & name) { m_name = name.toLower(); }
	void setCategory(NFileCategory_n::FileCategory fc) { m_category = fc; }
	void setShared(bool shared) { m_shared = shared; }

	bool isValid() const;

	bool operator==(const NFileSuffix & fileSuffix) const;
private:
	QString                          m_name;
	NFileCategory_n::FileCategory   m_category;
	bool                             m_shared;

};

class NFileSuffixList: public QHash<QString, NFileSuffix>
{
public:
	NFileSuffix category(const QFileInfo & fi) const;
	void dump() const;
	const QStringList suffixList(NFileCategory_n::FileCategory fc) const;
	const QStringList toDirNameFilters() const;
};

#endif // N_FILE_SUFFIX_H

