/*
 *n_dir.cpp - dirs class
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

#include <QVariant>

#include "n_convert.h"

#include "n_dir.h"

NDir::NDir(const QString & path, bool recursive, bool shared)
{
	m_path = path;
	m_dir.setPath(path);
	m_recursive = recursive;
	m_shared = shared;
}
	
bool NDir::recursive() const
{
	return m_recursive;
}

bool NDir::shared() const
{
	return m_shared;
}

const QDir & NDir::dir() const
{
	return m_dir;
}

const QString & NDir::path() const
{
	return m_path;
}

bool NDir::operator==(const NDir & dir) const
{
	return m_path == dir.m_path &&
		m_recursive == dir.m_recursive &&
		m_shared == dir.m_shared;
}

bool NDir::exists() const
{
	return m_dir.exists();
}

/****************************************************************
* NDirList
****************************************************************/
const NDirHash NDirList::getNotShared() const
{
	NDirHash hashList;
	for(int i = 0; i < count(); i++){
		NDir dir = at(i);
		if (dir.shared())
			continue;
		hashList[dir.path()] = dir;
	}
	return hashList;
}
