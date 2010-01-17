/*
 * n_dir_analyzer.cpp - file sharing controler
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
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <QFileInfoList>

// App
#include "n_database.h"
#include "n_file_hash.h"
#include "n_log.h"
#include "n_convert.h"

#include "n_hasher_thread.h"


NHasherThread::NHasherThread(QObject * parent)
	:NThread(parent)
{
}

void NHasherThread::run()
{
	NLOGM("NServer", tr("Hashing files..."));
	Q_ASSERT(NDB.beginTransaction());
	forever	{
		if (isStopping())
			break;

		if (!hashFile())
			break;
	}
	NDB.commitTransaction();
	NLOGM("NServer", tr("Hashing done."));
	NLOGM("NServer", tr("%1 data shared").arg(NConvert_n::byteToHuman(NDB.sharedSize())));
}

bool NHasherThread::hashFile()
{
	// Get a non hashed file in database
	const QFileInfo fi = NDB.fileToHash();
	if (!fi.exists())
	{
		Q_ASSERT_X(fi.fileName().isEmpty(), "hashFile", "invalid filename");
		NDB.removeDeletedFiles();
		NDB.removeDeletedDuplicatedFiles();
		emit hashingDone();
		return false;
	}

	NFileHash fileHash(fi);
	QString hash = fileHash.hash();
	Q_ASSERT_X(!hash.isEmpty(),  "NHasherThread::hashFile", "hash.isEmpty()");

	NDB.setFileHash(fi, hash);
	return true;
}
