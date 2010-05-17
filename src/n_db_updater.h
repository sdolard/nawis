/*
 * n_db_updater.h - Database updater
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

#ifndef N_DB_UPDATER_H
#define N_DB_UPDATER_H

//Qt
#include <QString>

// App
#include "n_thread.h"
#include "n_dir.h"
#include "n_file_suffix.h"
#include "n_dir_watcher.h"

class NDbUpdaterThread: public NThread
{
    Q_OBJECT
public:
    NDbUpdaterThread(const NDirWatcherThreadItems & dirs, QObject * parent = 0 );

protected:
    void run();

private:
    NDirWatcherThreadItems m_dirs;
    NFileSuffixList        m_fileSuffixes;
    QStringList            m_fileNameFilters;
    QDir                   m_dir;

    void updateDB();
    void parseFiles(const QString & path, const QString & rootPath);
    void parseSharedFiles();
};

#endif // N_DB_UPDATER_H

