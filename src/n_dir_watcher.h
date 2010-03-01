/*
 * kn_dir_watcher.cpp - file sharing watcher
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

#ifndef N_DIR_WATCHER_H
#define N_DIR_WATCHER_H

// Qt
#include <QList>
#include <QString>
#include <QByteArray>
#include <QMutex>
#include <QTime>
#include <QStringList>
#include <QHash>
#include <QCryptographicHash>
#include <QDir>

// App
#include "n_thread.h"
#include "n_dir.h"
#include "n_file_suffix.h"

// NDirWatcherThreadItem
class NDirWatcherThreadItem
{
    friend class NDirWatcherThread;
public:
    NDirWatcherThreadItem(const QString & path, const QString & rootPath);

    QByteArray hash() const;
    QByteArray previousHash() const;
    QString path() const;
    QString rootPath() const;

    bool hasChanged() const;
    bool exists() const;

private:
    QByteArray          m_hash;
    QByteArray          m_previousHash;
    QString             m_path;
    QString             m_rootPath;
};

// NDirWatcherThreadItems
class NDirWatcherThreadItems: public QHash<QString/*path*/, NDirWatcherThreadItem>
{
public:
    void addDir(const QString & path, const QString & rootPath);
};

// NDirWatcherThread
class NDirWatcherThread:  public NThread
{
    Q_OBJECT
public:
    NDirWatcherThread(QObject * parent = 0);
    ~NDirWatcherThread();

protected:
    void run();

private:
    NDirWatcherThreadItems m_dirs;
    QByteArray              m_hash;
    QCryptographicHash     *m_hasher;
    QDir                    m_dir;
    NDirList               m_sharedDirectories;
    NDirHash               m_notSharedDirs;
    NFileSuffixList        m_fileSuffixes;

    void parseSharedDirs();
    void parseDir(const QString & path, bool recursive, const QString & rootPath);
    void updateHash();
    void updateItemHash(NDirWatcherThreadItem & item);

signals:
    void hash(QString hash, NDirWatcherThreadItems dirWatcherThreadItems);
};

#endif // N_DIR_WATCHER_H
