/*
 * n_dir_watcher.cpp - file watcher
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

// Qt
#include <QFileInfo>
#include <QQueue>
#include <QMutexLocker>

// App
#include "n_log.h"
#include "n_convert.h"
#include "n_config.h"

#include "n_dir_watcher.h"

NDirWatcherThreadItem::NDirWatcherThreadItem(const QString & path, const QString & rootPath)
    :m_path(path), m_rootPath(rootPath)
{
}

QString NDirWatcherThreadItem::path() const
{
    return m_path;
}

QString NDirWatcherThreadItem::rootPath() const
{
    return m_rootPath;
}

bool NDirWatcherThreadItem::hasChanged() const
{
    return m_hash != m_previousHash;
}

QByteArray NDirWatcherThreadItem::hash() const
{
    return m_hash;
}

QByteArray NDirWatcherThreadItem::previousHash() const
{
    return m_previousHash;
}

bool NDirWatcherThreadItem::exists() const
{
    return QFile::exists(m_path);
}

/*******************************************************************************
 NDirWatcherThreadItems
 *******************************************************************************/
void NDirWatcherThreadItems::addDir(const QString & path, const QString & rootPath)
{
    if (contains(path))
        return;
    NDirWatcherThreadItem item(path, rootPath);
    insert(path, item);
};


/*******************************************************************************
 NDirWatcherThread
 *******************************************************************************/
NDirWatcherThread::NDirWatcherThread(QObject * parent)
    :NThread(parent)
{
    m_sharedDirectories = NCONFIG.sharedDirectories();
    m_notSharedDirs = m_sharedDirectories.getNotShared();
    m_fileSuffixes = NCONFIG.fileSuffixes();

    m_hasher = new QCryptographicHash(QCryptographicHash::Md5);
}

NDirWatcherThread::~NDirWatcherThread()
{
    delete m_hasher;
}

void NDirWatcherThread::run()
{
    logMessage("Server", "Looking for files modification...");
    parseSharedDirs();

    if (isStopping())
        return;

    updateHash();

    if (isStopping())
        return;

    emit hash(m_hash.toHex(), m_dirs);

    logMessage("Server", "Looking for files modification finished.");
}

void NDirWatcherThread::parseSharedDirs()
{
    //logDebug("NDirWatcherThread", "parseSharedDirs start");

    for(int i = 0; i < m_sharedDirectories.count(); i++)
    {
        const NDir & dir = m_sharedDirectories[i];
        if (isStopping())
            break;

        if (!dir.shared())
            continue;

        if (!dir.dir().exists())
            continue;

        parseDir(dir.dir().absolutePath(), dir.recursive(), dir.dir().absolutePath());
    }
    //logDebug("NDirWatcherThread", "parseSharedDirs stop");
}

void NDirWatcherThread::parseDir(const QString & path, bool recursive, const QString & rootPath)
{
    QDir d(path);
    if (!d.exists() || d.dirName() == "." || d.dirName() == "..")
        return;

    QString absolutePath = d.absolutePath();
    if (m_notSharedDirs.contains(absolutePath))
    {	// Excluded dir, we don't add it in database
        NDir dir = m_notSharedDirs[absolutePath];
        if (dir.recursive()) // Recursive exclusion
            return;
        // but we can add children in database
    } else {
        // Include dir
        QDir dir(rootPath);
        dir.cdUp();
        m_dirs.addDir(absolutePath, dir.absolutePath());
    }

    if (!recursive)
        return;

    d.setFilter(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QFileInfoList list = d.entryInfoList();
    for(int i = 0; i < list.count(); i++)
    {
        const QFileInfo & fi = list[i];
        if (isStopping())
            break;
        //qDebug(qPrintable(fi.absoluteFilePath()));
        parseDir(fi.absoluteFilePath(), recursive, rootPath); // absoluteFilePath because dir is considered has a file
    }
}

void NDirWatcherThread::updateHash()
{
    m_dir.setNameFilters(NCONFIG.fileSuffixes().toDirNameFilters());
    m_dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QCryptographicHash hash(QCryptographicHash::Md5);
    QMutableHashIterator<QString, NDirWatcherThreadItem> i(m_dirs);
    while (i.hasNext()) {
        i.next();

        if (isStopping())
            break;

        NDirWatcherThreadItem item = i.value();
        if (!item.exists())
        {
            i.remove();
            continue;
        }
        updateItemHash(item);
        i.setValue(item);
        hash.addData(item.hash());
        hash.addData(m_hash);
        m_hash = hash.result();
    }
}

void NDirWatcherThread::updateItemHash(NDirWatcherThreadItem & item)
{
    item.m_previousHash = item.m_hash;
    item.m_hash.clear();

    m_dir.setPath(item.m_path);
    if (!m_dir.exists())
        return;
    QFileInfoList list = m_dir.entryInfoList();
    for(int i = 0; i < list.count(); i++)
    {
        const QFileInfo & fi = list[i];
        NFileSuffix suffix = m_fileSuffixes.category(fi); // Optimized
        if (!suffix.isValid())
            continue;
        if (!suffix.shared())
            continue;

        m_hasher->reset();
        m_hasher->addData(fi.lastModified().toString().toUtf8()); // content change
        //if size changed, lastModified has changed to
        m_hasher->addData(fi.fileName().toUtf8()); // name change
        m_hasher->addData(item.m_hash); // previous hash (we get a hash for the directory, not for the file)
        item.m_hash = m_hasher->result();
    }
}
