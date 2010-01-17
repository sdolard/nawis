/*
 * n_dir.h - dirs class
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
 
#ifndef N_DIR_H
#define N_DIR_H
                 
// Qt
#include <QString>
#include <QHash>                           
#include <QDir>
#include <QList>

class NDir {
public:
	NDir() {};
	NDir(const QString & path, bool recursive, bool shared);
	
	bool recursive() const;  //default is true 
	bool shared() const;     //default is true
	const QDir & dir() const;
	const QString & path() const;
	
	bool operator==(const NDir & dir) const;
	
	bool exists() const;
private:
	QDir    m_dir;
	bool    m_recursive;  //default is true 
	bool    m_shared;     //default is true
	QString m_path;
};

class NDirHash: public QHash<QString, NDir>
{
};

class NDirList: public QList<NDir>
{
public:
	const NDirHash getNotShared() const;
};

#endif //N_DIR_H

