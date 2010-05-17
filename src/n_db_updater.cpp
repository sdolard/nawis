/*
 * n_db_updater.cpp - Database updater
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
#include "n_config.h"
#include "n_database.h"
#include "n_convert.h"
#include "n_log.h"

#include "n_db_updater.h"


NDbUpdaterThread::NDbUpdaterThread(const NDirWatcherThreadItems & dirs, QObject * parent)
    :NThread(parent)
{
    m_dirs = dirs;
    m_fileSuffixes = NCONFIG.fileSuffixes();
}

void NDbUpdaterThread::run()
{
    Q_ASSERT_X(NDB.beginTransaction(), "NDbUpdaterThread::run()", "NDB.beginTransaction()");
    NLOGM("Server", tr("Updating database..."));
    updateDB();
    NDB.commitTransaction();
    NLOGM("Server", tr("Database update done."));
}


void NDbUpdaterThread::updateDB()
{
    // Delete flag
    NDB.setFilesAsDeleted();
    NDB.setDuplicatedFilesAsDeleted();

    parseSharedFiles();
}

void NDbUpdaterThread::parseSharedFiles()
{
    NLOGD("NDbUpdaterThread", "parseSharedFiles start");
    m_dir.setNameFilters(NCONFIG.fileSuffixes().toDirNameFilters());
    m_dir.setFilter(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach(NDirWatcherThreadItem item, m_dirs) // todo: use QHashIterator
    {
        parseFiles(item.path(), item.rootPath());

        if (isStopping())
            break;
    }
    NLOGD("NDbUpdaterThread", "parseSharedFiles stop");
}

void NDbUpdaterThread::parseFiles(const QString & path, const QString & rootPath)
{
    m_dir.setPath(path);

    if (!m_dir.exists() || m_dir.dirName() == "." || m_dir.dirName() == "..")
        return;

    QFileInfoList list = m_dir.entryInfoList();
    for(int i = 0; i < list.count(); i++)
    {
        const QFileInfo & fi = list[i];
        if (isStopping())
            break;

        NFileSuffix suffix = m_fileSuffixes.category(fi); // Optimized
        if (!suffix.isValid())
        {
            //NLOGM("Not managed file", fi.fileName());
            continue;
        }
        if (!suffix.shared())
        {
            //qDebug("!ext.shared");
            continue;
        }
        NDB.addFile(fi, suffix, rootPath); // This flag file has not deleted if exists
    }
}

