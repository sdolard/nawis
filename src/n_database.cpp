/*
 * n_database.cpp - application database
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
#include <QCoreApplication>
#include <QSqlError>
#include <QMutexLocker>
#include <QDateTime>
#include <QSqlRecord>
#include <QSqlDriver>

// App
#include "n_path.h"
#include "n_file_category.h"
#include "n_file_suffix.h"
#include "n_config.h"
#include "n_log.h"
#include "n_convert.h"

#include "n_database.h"

NDatabase * NDatabase::m_instance = NULL;

NDatabase & NDatabase::instance()
{
    if (m_instance == NULL)
        m_instance = new NDatabase();
    return *m_instance;
}

void NDatabase::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NDatabase::NDatabase()
{
    QMutexLocker locker(&m_dbMutex);
    m_transactionPending = false;

    m_db = QSqlDatabase::addDatabase("QSQLITE");

    m_db.setDatabaseName(QString("%1%2.dbg").arg(NPath_n::config()).
                         arg(QCoreApplication::applicationName()));

    if (!m_db.open())
    {
        qDebug("NDatabase: m_db.open() failed %s", qPrintable(m_db.lastError().text()));
        Q_ASSERT(false);
    }
    createTables();
}

NDatabase::~NDatabase()
{
    abortTransaction();
    m_db.close();
}

void NDatabase::createCategoriesTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS categories (" \
            "id INTEGER PRIMARY KEY NOT NULL," \
            "name TEXT UNIQUE NOT NULL" \
            ")"))
        debugLastQuery("categories table creation failed", query);

    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_category_name ON categories(name)"))
        debugLastQuery("idx_category_name creation failed", query);


    // Default values
    if(!query.prepare("INSERT INTO categories (id, name) VALUES(:id, :name)"))
    {
        debugLastQuery("createDefautValues prepare failed", query);
        return;
    }

    for(NFileCategory_n::FileCategory fc = NFileCategory_n::fcAll; fc <= NFileCategory_n::fcDocument; fc++)
    {
        query.bindValue(":id", NFileCategory_n::fileCategoryId(fc));
        query.bindValue(":name", NFileCategory_n::fileCategoryName(fc));

        /*#ifdef DEBUG
                 if (!query.exec())
                 debugLastQuery("createDefautValues failed", query);
                 #else*/
        query.exec();
        //#endif //DEBUG
    }
}

void NDatabase::createMetadataTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS metadata ("\
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"\
            "file_id INTEGER UNIQUE NOT NULL,"\
            "artist TEXT,"\
            "comment TEXT,"\
            "year INTEGER,"\
            "album TEXT,"\
            "title TEXT,"\
            "genre TEXT,"\
            "trackNumber INTEGER,"\
            "duration INTEGER,"\
            "hasID3Picture BOOLEAN DEFAULT 0 NOT NULL,"\
            "dateTimeOriginal TIMESTAMP,"\
            "copyright TEXT,"\
            "width INTEGER,"\
            "height INTEGER,"\
            "make TEXT,"\
            "model TEXT,"\
            "city TEXT,"\
            "provinceState TEXT,"\
            "country TEXT,"\
            "latitude TEXT,"\
            "longitude TEXT,"\
            "altitude INTEGER"\
            ")"))
        debugLastQuery("metadata table creation failed", query);


    if (!query.exec(
            "CREATE TRIGGER IF NOT EXISTS fkd_metadata_files_id " \
            "BEFORE DELETE ON files "\
            "FOR EACH ROW BEGIN "\
            "  DELETE from metadata WHERE file_id = OLD.id; "\
            "END; "))
        debugLastQuery("files CREATE TRIGGER fkd_metadata_files_id failed", query);
}

void NDatabase::createFilesTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS files (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fileName TEXT NOT NULL," \
            "relativePath TEXT NOT NULL," \
            "absoluteFilePath TEXT UNIQUE NOT NULL," \
            "category_id INTEGER NOT NULL," \
            "metadata_id INTEGER "\
            "  CONSTRAINT fk_metadata_id REFERENCES metadata(id) ON DELETE CASCADE,"\
            "hash TEXT,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL," \
            "added TIMESTAMP NOT NULL,"
            "size INTEGER NOT NULL,"
            "lastModified TIMESTAMP NOT NULL"
            ")"))
        debugLastQuery("files table creation failed", query);


    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_file_index "\
                    "ON files(absoluteFilePath, lastModified)"))
        debugLastQuery("idx_file_index creation failed", query);


    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_file_hash "\
                    "ON files(hash)"))
        debugLastQuery("idx_file_hash creation failed", query);
}

void NDatabase::createDuplicatedFilesTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS duplicated_files (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fileName TEXT NOT NULL," \
            "relativePath TEXT NOT NULL," \
            "absoluteFilePath TEXT UNIQUE NOT NULL," \
            "category_id INTEGER NOT NULL," \
            "hash TEXT NOT NULL,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL," \
            "added TIMESTAMP NOT NULL,"
            "size INTEGER NOT NULL,"
            "lastModified TIMESTAMP NOT NULL"
            ")"))
        debugLastQuery("duplicated_files table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_duplicated_file_index "\
                    "ON duplicated_files(hash, absoluteFilePath)"))
        debugLastQuery("idx_duplicated_file_index creation failed", query);
}

void NDatabase::createUsersTable()
{
    /*QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS duplicated_files (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fileName TEXT NOT NULL," \
            "relativePath TEXT NOT NULL," \
            "absoluteFilePath TEXT UNIQUE NOT NULL," \
            "category_id INTEGER NOT NULL," \
            "hash TEXT NOT NULL,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL," \
            "added TIMESTAMP NOT NULL,"
            "size INTEGER NOT NULL,"
            "lastModified TIMESTAMP NOT NULL"
            ")"))
        debugLastQuery("duplicated_files table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_duplicated_file_index "\
                    "ON duplicated_files(hash, absoluteFilePath)"))
        debugLastQuery("idx_duplicated_file_index creation failed", query);*/
}

void NDatabase::createMusicAlbumTable()
{
    QSqlQuery query(m_db);

    // TODO: delete entries when no more used
    // Delete after update
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_album (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "name TEXT," \
            "music_album_cover_id INTEGER,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL" \
            ")"))
        debugLastQuery("music_album table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_album_index "\
                    "ON music_album(name)"))
        debugLastQuery("idx_music_album_index creation failed", query);
}

void NDatabase::createMusicCoverTable()
{
    QSqlQuery query(m_db);

    // TODO: delete entries when no more used
    // Delete when file hash reference is deleted
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_album_cover (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "music_album_id INTEGER NOT NULL," \
            "file_hash text NOT NULL," \
            "front BOOLEAN DEFAULT 1 NOT NULL"\
            ")"))
        debugLastQuery("music_album_cover table creation failed", query);
}

void NDatabase::createMusicAlbumTitleTable()
{
    QSqlQuery query(m_db);

    // TODO: delete entries when no more used
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_album_title (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "music_album_id INTEGER NOT NULL," \
            "file_hash text NOT NULL"\
            ")"))
        debugLastQuery("music_album_title table creation failed", query);
}

void NDatabase::createMusicTitleTable()
{
    QSqlQuery query(m_db);

    // TODO: delete entries when no more used
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_title (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "file_hash TEXT NOT NULL,"\
            "music_author_id INTEGER NOT NULL," \
            "music_genre_id INTEGER NOT NULL" \
            ")"))
        debugLastQuery("music_title table creation failed", query);
}

void NDatabase::createMusicAuthorTable()
{
    QSqlQuery query(m_db);

    // TODO: delete entries when no more used
    // Delete on update
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_author (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "name TEXT UNIQUE,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL" \
            ")"))
        debugLastQuery("music_author table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_author_index "\
                    "ON music_author(name)"))
        debugLastQuery("idx_music_author_index creation failed", query);
}

void NDatabase::createMusicGenreTable()
{
    QSqlQuery query(m_db);

    // TODO: delete entries when no more used
    // Delete on update
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_genre (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "name TEXT,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL" \
            ")"))
        debugLastQuery("music_genre table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_genre_index "\
                    "ON music_genre(name)"))
        debugLastQuery("idx_music_genre_index creation failed", query);
}


void NDatabase::createTables()
{
    // Order is important!
    createCategoriesTable();
    createFilesTable();
    createMetadataTable();
    createDuplicatedFilesTable();
    createUsersTable();

    // Music
    createMusicAlbumTable();
    createMusicAlbumTitleTable();
    createMusicTitleTable();
    createMusicAuthorTable();
    createMusicGenreTable();
    createMusicCoverTable();
}

void NDatabase::debugLastQuery(const QString & msg, const QSqlQuery & query)
{
    NLOGDD("NDatabase", "Data base error:");
    NLOGDD("NDatabase", QString("  %1").arg(msg));
    NLOGDD("NDatabase", QString("  %1").arg(query.lastError().text()));
    NLOGDD("NDatabase", QString("  %1").arg(query.lastQuery()));
    Q_ASSERT(false);
}

bool NDatabase::beginTransaction()
{
    QMutexLocker locker(&m_dbMutex);
    if (m_transactionPending)
        return false;

    if (!m_db.transaction())
        return false;
    m_transactionPending = true;
    return true;
}

bool NDatabase::commitTransaction()
{
    QMutexLocker locker(&m_dbMutex);
    if (!m_transactionPending)
        return false;
    if (!m_db.commit())
        return false;
    m_transactionPending = false;
    return true;
}

bool NDatabase::abortTransaction()
{
    QMutexLocker locker(&m_dbMutex);
    if (!m_transactionPending)
        return false;
    if (m_db.rollback())
        return false;
    m_transactionPending = false;
    //qDebug("NDatabase::abortTransaction");
    return true;
}

QString NDatabase::lastDbError()
{
    return m_db.lastError().text();
}

bool NDatabase::setFilesAsDeleted()
{
    QSqlQuery query(m_db);
    if (!query.exec("UPDATE files SET deleted = 1"))
    {
        debugLastQuery("setFilesAsDeleted failed", query);
        return false;
    }
    return true;
}

bool NDatabase::setDuplicatedFilesAsDeleted()
{
    QSqlQuery query(m_db);
    if (!query.exec("UPDATE duplicated_files SET deleted = 1"))
    {
        debugLastQuery("setDuplicatedFilesAsDeleted failed", query);
        return false;
    }
    return true;
}

bool NDatabase::setFileAsNotDeleted(const QString & absoluteFilePath, const QDateTime & lastModified)
{
    QSqlQuery query(m_db);
    if(!query.prepare("UPDATE files SET deleted = 0 "\
                      "WHERE absoluteFilePath = :absoluteFilePath "\
                      "AND lastModified = :lastModified")) // same file, no update hash needed
    {
        debugLastQuery("setFileAsNotDeleted prepare failed", query);
        return false;
    }

    query.bindValue(":absoluteFilePath", absoluteFilePath);
    query.bindValue(":lastModified", lastModified.toString(SQLITE_DATETIME));
    if (!query.exec())
    {
        debugLastQuery("setFileAsNotDeleted failed", query);
        return false;
    }
    return true;
}

bool NDatabase::setDuplicatedFileAsNotDeleted(const QFileInfo & fi)
{
    if (!fi.isFile())
        return false;

    QSqlQuery query(m_db);
    if (!query.prepare("UPDATE duplicated_files SET deleted = 0 "\
                       "WHERE absoluteFilePath = :absoluteFilePath "\
                       "AND lastModified = :lastModified")) // same file, no update hash needed
    {
        debugLastQuery("setDuplicatedFileAsNotDeleted prepare failed", query);
        return false;
    }
    query.bindValue(":absoluteFilePath", fi.absoluteFilePath());
    query.bindValue(":lastModified", fi.lastModified().toString(SQLITE_DATETIME));
    if (!query.exec())
    {
        debugLastQuery("setDuplicatedFileAsNotDeleted failed", query);
        return false;
    }
    return true;
}


bool NDatabase::removeDeletedFiles()
{
    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM files WHERE deleted = 1"))
    {
        debugLastQuery("removeDeletedFiles failed", query);
        return false;
    }
    return true;
}

bool NDatabase::removeDeletedDuplicatedFiles()
{
    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM duplicated_files WHERE deleted = 1"))
    {
        debugLastQuery("removeDeletedDuplicatedFiles failed", query);
        return false;
    }
    return true;
}

bool NDatabase::addFile(const QFileInfo & fi, const NFileSuffix & suffix, const QString & rootPath)
{
    if (!fi.isFile())
        return false;
    if (!suffix.isValid())
        return false;
    if (!suffix.shared())
        return false;

    QString absoluteFilePath = fi.absoluteFilePath();
    QDateTime lastModified = fi.lastModified();

    QSqlQuery query(m_db);
    if (!query.prepare("INSERT INTO files (fileName, relativePath, "\
                       "absoluteFilePath, category_id, added, size, lastModified) "\
                       "VALUES(:fileName, :relativePath, :absoluteFilePath, :category_id, "\
                       ":added, :size, :lastModified)"))
    {
        debugLastQuery("addFile prepare failed", query);
        return setFileAsNotDeleted(absoluteFilePath, lastModified);
    }

    query.bindValue(":fileName", fi.fileName());
    query.bindValue(":relativePath", fi.absolutePath().remove(rootPath));
    query.bindValue(":absoluteFilePath", absoluteFilePath);
    query.bindValue(":category_id", suffix.category());
    query.bindValue(":added", QDateTime::currentDateTime().toString(SQLITE_DATETIME));
    query.bindValue(":size", fi.size());
    query.bindValue(":lastModified", lastModified.toString(SQLITE_DATETIME));

    if (!query.exec())
    {
        // uncomment if needed
        //debugLastQuery("addFile failed", query);
        return setFileAsNotDeleted(absoluteFilePath, lastModified);
    }

    return true;
}

const QFileInfo NDatabase::fileToHash()
{
    QSqlQuery query(m_db);

    QString sql_filetoHash = "SELECT absoluteFilePath "\
                             "FROM files "\
                             "WHERE hash IS NULL "\
                             "AND deleted = 0 "\
                             "LIMIT 1 ";

    QString sql_duplicated = "SELECT files.absoluteFilePath "\
                             "FROM duplicated_files, files "\
                             "WHERE duplicated_files.hash = files.hash "\
                             "AND duplicated_files.absoluteFilePath = :absoluteFilePath "\
                             "AND files.deleted = 0 ";

    QString sql_deleteFiletoHash = "DELETE FROM files "\
                                   "WHERE absoluteFilePath = :absoluteFilePath ";

    while (true) {
        // we are looking for a file to hash
        if (!query.exec(sql_filetoHash))
        {
            debugLastQuery("fileToHash failed (1)", query);
            return QFileInfo();
        }

        if (!query.first())
            return QFileInfo(); // no more file to hash


        // we are looking for a file to hash referenced in duplicated file table
        // If it's exists, we are looking if "original file exists yet"
        // if not, it's a new file,
        // if it's exists, no need to hash the file, we delete the file in hash table
        // we flag duplicated file as not deleted
        // and looking for another file to hash until there is no more file to hash
        QFileInfo fiToHash(query.value(0).toString());
        query.clear();
        if (!query.prepare(sql_duplicated))
        {
            debugLastQuery("fileToHash failed (2)", query);
            return QFileInfo();
        }
        query.bindValue(":absoluteFilePath", fiToHash.absoluteFilePath());
        if (!query.exec())
        {
            debugLastQuery("fileToHash failed (3)", query);
            return QFileInfo();
        }
        if (!query.first())
            return fiToHash; // no duplicated file reference

        QFileInfo refFi(query.value(0).toString());
        if (!refFi.exists()) // Original file do not exists
            return fiToHash;

        // we delete the file in hash table and looking for another file to hash
        // until there is no more file to hash
        query.clear();
        if (!query.prepare(sql_deleteFiletoHash))
        {
            debugLastQuery("fileToHash failed (4)", query);
        }
        query.bindValue(":absoluteFilePath", fiToHash.absoluteFilePath());
        if (!query.exec())
        {
            debugLastQuery("fileToHash failed (5)", query);
        }

        // Update deleted status of duplicated file reference
        setDuplicatedFileAsNotDeleted(fiToHash);
    }
}

bool NDatabase::getFileWithNoMetadata(QString & absoluteFilePath, int & fileId)
{
    absoluteFilePath.clear();
    fileId = 0;

    QSqlQuery query(m_db);

    QString sql = "SELECT id, absoluteFilePath "\
                  "FROM files "\
                  "WHERE metadata_id IS NULL "\
                  "AND deleted = 0 "\
                  "LIMIT 1 ";

    // we are looking for a file
    if (!query.exec(sql))
    {
        debugLastQuery("getFileWithNoMetadata failed (1)", query);
        return false;
    }

    if (!query.first())
        return false; // no more file to hash

    fileId = query.value(0).toInt();
    absoluteFilePath = query.value(1).toString();
    return true;
}


bool NDatabase::setMetadata(int fileId, const NMetadata & metadata)
{
    if (fileId < 0)
        return false;

    QSqlQuery query(m_db);

    if (!query.prepare("INSERT INTO metadata (file_id, artist, comment, year, album, "\
                       "title, genre, trackNumber, duration,  dateTimeOriginal,"\
                       "copyright, width, height, make, model, latitude,"\
                       "longitude, altitude, city, provinceState, country, hasID3Picture) " \
                       "VALUES(:file_id, :artist, :comment, :year, :album, "\
                       ":title, :genre, :trackNumber, :duration,  :dateTimeOriginal,"\
                       ":copyright, :width, :height, :make, :model, :latitude,"\
                       ":longitude, :altitude, :city, :provinceState, :country, :hasID3Picture)"))
    {
        debugLastQuery("setMetadata prepare failed", query);
        return false;
    }

    query.bindValue(":file_id", fileId);
    query.bindValue(":artist", metadata.artist());
    query.bindValue(":comment", metadata.comment());
    query.bindValue(":year", metadata.year());
    query.bindValue(":album", metadata.album());
    query.bindValue(":title", metadata.title());
    query.bindValue(":genre", metadata.genre());
    query.bindValue(":trackNumber", metadata.trackNumber());
    query.bindValue(":duration", metadata.duration());
    query.bindValue(":hasID3Picture", metadata.hasID3Picture());
    query.bindValue(":dateTimeOriginal", metadata.dateTimeOriginal().toString(SQLITE_DATETIME));
    query.bindValue(":copyright", metadata.copyright());
    query.bindValue(":width", metadata.width());
    query.bindValue(":height", metadata.height());
    query.bindValue(":make", metadata.make());
    query.bindValue(":model", metadata.model());
    query.bindValue(":latitude", metadata.latitude());
    query.bindValue(":longitude", metadata.longitude());
    query.bindValue(":altitude", metadata.altitude());
    query.bindValue(":city", metadata.city());
    query.bindValue(":provinceState", metadata.provinceState());
    query.bindValue(":country", metadata.country());
    if (!query.exec())
    {
        debugLastQuery("setMetadata failed (0)", query);
        return false;
    }

    query.clear();
    if (!query.prepare("UPDATE files SET metadata_id=(SELECT id FROM metadata WHERE file_id=:file_id) WHERE id=:id"))
    {
        debugLastQuery("setMetadata prepare failed (1)", query);
        return false;
    }
    query.bindValue(":file_id", fileId);
    query.bindValue(":id", fileId);
    if (!query.exec())
    {
        debugLastQuery("setMetadata failed (1)", query);
        return false;
    }

    return true;
}

bool NDatabase::getFileList(QScriptEngine & se, QScriptValue & dataArray, const QStringList & searches, int start,
                            int limit, NFileCategory_n::FileCategory fc,
                            const QString & sort, const QString & dir)
{
    // TODO: do category only search

    QSqlQuery query(m_db);
    QString sql = "SELECT files.id id, files.fileName, files.relativePath, files.hash, "\
                  "files.added, files.size, files.category_id, files.lastModified, " \
                  "metadata.artist, metadata.comment, metadata.year, metadata.album, "\
                  "metadata.title, metadata.genre, metadata.trackNumber, metadata.duration, "\
                  "metadata.dateTimeOriginal, metadata.copyright, metadata.width, metadata.height, "\
                  "metadata.make, metadata.model, metadata.latitude, metadata.longitude, metadata.altitude, "\
                  "metadata.city, metadata.provinceState, metadata.country, metadata.hasID3Picture "\
                  "FROM files, metadata " \
                  "WHERE files.metadata_id = metadata.id "\
                  "AND files.hash <> '' ";

    if (fc != NFileCategory_n::fcAll)
        sql += "AND files.category_id = :category_id ";


    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (files.relativePath LIKE :relativePath%1 "\
                       "OR files.fileName LIKE :fileName%1 "
                       "OR metadata.artist LIKE :artist%1 "\
                       "OR metadata.comment LIKE :comment%1 "\
                       "OR metadata.album LIKE :album%1 "\
                       "OR metadata.title LIKE :title%1 "\
                       "OR metadata.genre LIKE :genre%1 "\
                       "OR metadata.make LIKE :make%1 "\
                       "OR metadata.city LIKE :city%1 "\
                       "OR metadata.provinceState LIKE :provinceState%1 "\
                       "OR metadata.country LIKE :country%1 "\
                       "OR metadata.model LIKE :model%1) ").arg(i);

    // Sort and limit
    sql += QString("ORDER BY %1 %2 LIMIT :limit OFFSET :offset").
           arg(stringToFileField(sort)).
           arg(stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        debugLastQuery("fileList prepare failed", query);
        return false;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":relativePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":fileName%1").arg(i), QString("%%1%").arg(s));

        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":comment%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(s));

        query.bindValue(QString(":make%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":model%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":city%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":provinceState%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":country%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":category_id", (NFileCategory_n::fileCategoryId(fc)));

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    //TMP  if (!query.exec())
    if (!query.exec())
    {
        debugLastQuery("fileList failed", query);
        return false;
    } else {
        NLOGD("NDatabase", QString("  %1").arg(query.lastQuery()));
    }

    // Files fields
    int fieldId = query.record().indexOf("id");
    int fieldFileName = query.record().indexOf("fileName");
    int fieldRelativePath = query.record().indexOf("relativePath");
    int fieldCategoryId = query.record().indexOf("category_id");
    int fieldHash = query.record().indexOf("hash");
    int fieldAdded = query.record().indexOf("added");
    int fieldSize = query.record().indexOf("size");
    int fieldLastModified = query.record().indexOf("lastModified");

    int fieldArtist = query.record().indexOf("artist");
    int fieldComment = query.record().indexOf("comment");
    int fieldYear = query.record().indexOf("year");
    int fieldAlbum = query.record().indexOf("album");
    int fieldTitle = query.record().indexOf("title");
    int fieldGenre = query.record().indexOf("genre");
    int fieldTrackNumber = query.record().indexOf("trackNumber");
    int fieldDuration = query.record().indexOf("duration");
    int hasID3Picture = query.record().indexOf("hasID3Picture");

    int fieldDateTimeOriginal = query.record().indexOf("dateTimeOriginal");
    int fieldCopyright = query.record().indexOf("copyright");
    int fieldWidth = query.record().indexOf("width");
    int fieldHeight = query.record().indexOf("height");
    int fieldMake = query.record().indexOf("make");
    int fieldModel = query.record().indexOf("model");
    int fieldLongitude = query.record().indexOf("longitude");
    int fieldLatitude = query.record().indexOf("latitude");
    int fieldAltitude = query.record().indexOf("altitude");

    int fieldCity = query.record().indexOf("city");
    int fieldProvinceState = query.record().indexOf("provinceState");
    int fieldCountry = query.record().indexOf("country");

    int i = 0;
    while (query.next()) {
        QScriptValue svfile = se.newObject();
        dataArray.setProperty(i, svfile);

        // Files field
        svfile.setProperty("id", query.value(fieldId).toInt());
        svfile.setProperty("fileName", query.value(fieldFileName).toString());
        svfile.setProperty("relativePath", query.value(fieldRelativePath).toString());
        NFileCategory_n::FileCategory category = NFileCategory_n::toFileCategory(query.value(fieldCategoryId).toInt());
        svfile.setProperty("category", NFileCategory_n::fileCategoryName(category));
        svfile.setProperty("hash", query.value(fieldHash).toString());
        svfile.setProperty("added", query.value(fieldAdded).toString());
        svfile.setProperty("size", query.value(fieldSize).toInt());
        svfile.setProperty("lastModified", query.value(fieldLastModified).toString());


        if (category == NFileCategory_n::fcMusic)
        {
            // Metadata field
            svfile.setProperty("artist", query.value(fieldArtist).toString());
            svfile.setProperty("comment", query.value(fieldComment).toString());
            svfile.setProperty("year", query.value(fieldYear).toInt());
            svfile.setProperty("album", query.value(fieldAlbum).toString());
            svfile.setProperty("title", query.value(fieldTitle).toString());
            svfile.setProperty("genre", query.value(fieldGenre).toString());
            svfile.setProperty("trackNumber", query.value(fieldTrackNumber).toInt());
            svfile.setProperty("duration", query.value(fieldDuration).toInt());
            svfile.setProperty("hasID3Picture", query.value(hasID3Picture).toString());
        }

        if (category == NFileCategory_n::fcPicture)
        {
            // Metadata
            svfile.setProperty("dateTimeOriginal", query.value(fieldDateTimeOriginal).toString());
            svfile.setProperty("copyright", query.value(fieldCopyright).toString());
            svfile.setProperty("width", query.value(fieldWidth).toInt());
            svfile.setProperty("height", query.value(fieldHeight).toInt());
            svfile.setProperty("make", query.value(fieldMake).toString());
            svfile.setProperty("model", query.value(fieldModel).toString());

            svfile.setProperty("longitude", query.value(fieldLongitude).toString());
            svfile.setProperty("latitude", query.value(fieldLatitude).toString());
            svfile.setProperty("altitude", query.value(fieldAltitude).toInt());

            svfile.setProperty("city", query.value(fieldCity).toString());
            svfile.setProperty("provinceState", query.value(fieldProvinceState).toString());
            svfile.setProperty("coutry", query.value(fieldCountry).toString());
        }
        i++;
    }
    return true;
}

int NDatabase::getFileListCount(const QStringList & searches,
                                NFileCategory_n::FileCategory fc)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT COUNT(*) "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND files.hash <> '' ";

    if (fc != NFileCategory_n::fcAll)
        sql += "AND files.category_id = :category_id ";

    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (files.relativePath LIKE :relativePath%1 "\
                       "OR files.fileName LIKE :fileName%1 "
                       "OR metadata.artist LIKE :artist%1 "\
                       "OR metadata.comment LIKE :comment%1 "\
                       "OR metadata.album LIKE :album%1 "\
                       "OR metadata.title LIKE :title%1 "\
                       "OR metadata.genre LIKE :genre%1 "\
                       "OR metadata.make LIKE :make%1 "\
                       "OR metadata.city LIKE :city%1 "\
                       "OR metadata.provinceState LIKE :provinceState%1 "\
                       "OR metadata.country LIKE :country%1 "\
                       "OR metadata.model LIKE :model%1) ").arg(i);


    if (!query.prepare(sql))
    {
        debugLastQuery("fileListCount prepare failed", query);
        return 0;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":relativePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":fileName%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":comment%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":make%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":model%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":city%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":provinceState%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":country%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":category_id", (NFileCategory_n::fileCategoryId(fc)));

    if (!query.exec())
    {
        debugLastQuery("fileListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}

bool NDatabase::getDuplicatedFileList(QScriptEngine & se, QScriptValue & dataArray,
                                      const QStringList & searches, int start,
                                      int limit, NFileCategory_n::FileCategory fc,
                                      const QString & sort, const QString & dir)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT duplicated_files.id, duplicated_files.fileName, "\
                  "duplicated_files.relativePath, duplicated_files.absoluteFilePath, "\
                  "duplicated_files.hash, duplicated_files.added, duplicated_files.size, "\
                  "duplicated_files.category_id, files.absoluteFilePath original_absoluteFilePath "\
                  "FROM files, duplicated_files "\
                  "WHERE files.hash = duplicated_files.hash "\
                  "AND files.hash <> '' ";

    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (duplicated_files.relativePath LIKE :duplicated_relativePath%1  OR "\
                       "duplicated_files.absoluteFilePath LIKE :duplicated_absoluteFilePath%1  OR "\
                       "duplicated_files.fileName LIKE :duplicated_fileName%1  OR "\
                       "files.relativePath LIKE :relativePath%1  OR "\
                       "files.fileName LIKE :fileName%1) ").arg(i);

    if (fc != NFileCategory_n::fcAll)
        sql += "AND duplicated_files.category_id = :category_id ";

    // Sort and limit
    sql += QString("ORDER BY duplicated_files.%1 %2 LIMIT :limit OFFSET :offset").
           arg(stringToFileField(sort)).
           arg(stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        debugLastQuery("duplicatedFileList prepare failed", query);
        return false;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":duplicated_relativePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_absoluteFilePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_fileName%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":relativePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":fileName%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":category_id", (NFileCategory_n::fileCategoryId(fc)));

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("duplicatedFileList failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
    int fieldFileName = query.record().indexOf("fileName");
    int fieldRelativePath = query.record().indexOf("relativePath");
    int fieldAbsoluteFilePath = query.record().indexOf("absoluteFilePath");
    int fieldCategoryId = query.record().indexOf("category_id");
    int fieldHash = query.record().indexOf("hash");
    int fieldAdded = query.record().indexOf("added");
    int fieldSize = query.record().indexOf("size");
    int fieldOriginalAbsoluteFilePath = query.record().indexOf("original_absoluteFilePath");

    int i = 0;
    while (query.next()) {
        QScriptValue svfile = se.newObject();
        dataArray.setProperty(i, svfile);

        // Files field
        svfile.setProperty("id", query.value(fieldId).toInt());
        svfile.setProperty("fileName", query.value(fieldFileName).toString());
        svfile.setProperty("relativePath", query.value(fieldRelativePath).toString());
        svfile.setProperty("absoluteFilePath", query.value(fieldAbsoluteFilePath).toString());
        NFileCategory_n::FileCategory fc = NFileCategory_n::toFileCategory(query.value(fieldCategoryId).toInt());
        svfile.setProperty("category", NFileCategory_n::fileCategoryName(fc));
        svfile.setProperty("hash", query.value(fieldHash).toString());
        svfile.setProperty("added", query.value(fieldAdded).toString());
        svfile.setProperty("size", query.value(fieldSize).toInt());
        svfile.setProperty("originalAbsoluteFilePath", query.value(fieldOriginalAbsoluteFilePath).toString());

        i++;
    }
    return true;
}


int NDatabase::getDuplicatedFileListCount(const QStringList & searches, NFileCategory_n::FileCategory fc)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT COUNT(*) " \
                  "FROM files, duplicated_files "\
                  "WHERE files.hash = duplicated_files.hash "\
                  "AND files.hash <> '' ";

    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (duplicated_files.relativePath LIKE :duplicated_relativePath%1  OR "\
                       "duplicated_files.absoluteFilePath LIKE :duplicated_absoluteFilePath%1  OR "\
                       "duplicated_files.fileName LIKE :duplicated_fileName%1  OR "\
                       "files.relativePath LIKE :relativePath%1  OR "\
                       "files.fileName LIKE :fileName%1) ").arg(i);

    if (fc != NFileCategory_n::fcAll)
        sql += QString("AND duplicated_files.category_id = :category_id ");

    if (!query.prepare(sql))
    {
        debugLastQuery("duplicatedFileListCount prepare failed", query);
        return 0;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":duplicated_relativePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_absoluteFilePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_fileName%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":relativePath%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":fileName%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":category_id", (NFileCategory_n::fileCategoryId(fc)));


    if (!query.exec())
    {
        debugLastQuery("duplicatedFileListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}


QFileInfo NDatabase::file(const QString & fileHash)
{
    if (fileHash.isEmpty())
        return QFileInfo();

    QSqlQuery query(m_db);
    if (!query.prepare("SELECT absoluteFilePath, hash, added, size, category_id " \
                       "FROM files "\
                       "WHERE hash =:hash"))
    {
        debugLastQuery("fileList prepare failed", query);
        return QFileInfo();
    }
    query.bindValue(":hash", fileHash);
    if (!query.exec())
    {
        debugLastQuery("fileList failed", query);
        return QFileInfo();
    }

    if (!query.first())
        return QFileInfo();

    return QFileInfo(query.value(0).toString());
}

bool NDatabase::isDuplicatedFile(const QString & hash, const QFileInfo & newFi)
{
    // called in setFileHash
    QSqlQuery query(m_db);
    if (!query.prepare("SELECT fileName,relativePath,absoluteFilePath FROM files WHERE hash = :hash"))
    {
        debugLastQuery("isDuplicatedFileprepare failed (1)", query);
        return false;
    }
    query.bindValue(":hash", hash);
    if (!query.exec())
    {
        debugLastQuery("isDuplicatedFile failed (1)", query);
        return false;
    }
    if (!query.first())
        return false;
    QFileInfo fi(query.value(2).toString());
    if (!fi.exists())
        return false;

    QString relativeFilePath = QDir::toNativeSeparators(QString("%1/%2").
                                                        arg(query.value(1).toString()).
                                                        arg(query.value(0).toString()));
    QString absoluteFilePath = query.value(2).toString();
    QString rootPath = absoluteFilePath.remove(relativeFilePath);

    query.clear();
    // we add it in duplicated files table
    if(!query.prepare("INSERT INTO duplicated_files (fileName, relativePath, "\
                      "absoluteFilePath, category_id, added, size, lastModified, hash) "\
                      "VALUES(:fileName, :relativePath, :absoluteFilePath, :category_id, "\
                      ":added, :size, :lastModified, :hash)"))
    {
        debugLastQuery("isDuplicatedFile prepare failed (2)", query);
        return false;
    }

    query.bindValue(":fileName", newFi.fileName());
    query.bindValue(":relativePath", newFi.absolutePath().remove(rootPath));
    query.bindValue(":absoluteFilePath", newFi.absoluteFilePath());
    query.bindValue(":category_id",
                    NFileCategory_n::fileCategoryId(NCONFIG.fileSuffixes().category(newFi).category()));
    query.bindValue(":added", QDateTime::currentDateTime().toString(SQLITE_DATETIME));
    query.bindValue(":size", newFi.size());
    query.bindValue(":lastModified", newFi.lastModified().toString(SQLITE_DATETIME));
    query.bindValue(":hash", hash);

    if (!query.exec())
    {
        // uncomment if needed
        //debugLastQuery("isDuplicatedFile failed (2)", query); //If file alreay exist, insert failed, it's normal.
        setDuplicatedFileAsNotDeleted(newFi);
    }

    query.clear();

    if (!query.prepare("DELETE FROM files WHERE absoluteFilePath = :absoluteFilePath"))
    {
        debugLastQuery("isDuplicatedFile prepare failed (3)", query);
        return false;
    }
    query.bindValue(":absoluteFilePath", newFi.absoluteFilePath());
    if (!query.exec())
    {
        debugLastQuery("isDuplicatedFile failed (3)", query);
        return false;
    }

    return true;
}

void NDatabase::setFileHash(const QFileInfo & fi, const QString & hash)
{
    Q_ASSERT(fi.exists());
    Q_ASSERT(!hash.isEmpty());

    if (!fi.isFile())
        return;
    QString addedDateTime;
    QSqlQuery query(m_db);

    // We are looking for a previous file with same hash in order to not update
    // added dateTime
    if (!query.prepare("SELECT added FROM files WHERE hash = :hash"))
    {
        debugLastQuery("setFileHash prepare failed (1)", query);
        return;
    }
    query.bindValue(":hash", hash);
    if (!query.exec())
    {
        debugLastQuery("setFileHash failed (1)", query);
        return;
    }

    if (query.first())
    {
        addedDateTime = query.value(0).toString();
        query.clear();

        // Looking for files exists yet, doubled file

        if (isDuplicatedFile(hash, fi))
            return;

        // we delete previous instance of this hash
        if (!query.prepare("DELETE FROM files WHERE hash = :hash"))
        {
            debugLastQuery("setFileHash prepare failed (2)", query);
            return;
        }
        query.bindValue(":hash", hash);
        if (!query.exec())
        {
            debugLastQuery("setFileHash failed (2)", query);
            return;
        }
        query.clear();
        if (!query.prepare("UPDATE files SET hash = :hash, added = :added "\
                           "WHERE absoluteFilePath = :absoluteFilePath "\
                           "AND lastModified = :lastModified"))
        {
            debugLastQuery("setFileHash prepare failed (3)", query);
            return;
        }
        query.bindValue(":hash", hash);
        query.bindValue(":added", addedDateTime);
        query.bindValue(":lastModified", fi.lastModified().toString(SQLITE_DATETIME));
        query.bindValue(":absoluteFilePath", fi.absoluteFilePath());
        if (!query.exec())
        {
            debugLastQuery("setFileHash failed (3)", query);
            return;
        }
        return;
    }

    query.clear();
    if (!query.prepare("UPDATE files SET hash = :hash "\
                       "WHERE absoluteFilePath = :absoluteFilePath "\
                       "AND lastModified = :lastModified"))
    {
        debugLastQuery("setFileHash prepare failed (4)", query);
        return;
    }
    query.bindValue(":hash", hash);
    query.bindValue(":lastModified", fi.lastModified().toString(SQLITE_DATETIME));
    query.bindValue(":absoluteFilePath", fi.absoluteFilePath());
    if (!query.exec())
    {
        debugLastQuery("setFileHash failed (4)", query);
        return;
    }
    return;
}

qint64 NDatabase::sharedSize()
{
    QSqlQuery query(m_db);
    if (!query.exec("SELECT sum(size) FROM files"))
    {
        debugLastQuery("sizeShared failed", query);
        return 0;
    }
    if (!query.first())
        return 0;
    return query.value(0).toLongLong();
}

qint64 NDatabase::sizeOfDuplicatedFiles()
{
    QSqlQuery query(m_db);
    if (!query.exec("SELECT sum(size) FROM duplicated_files"))
    {
        debugLastQuery("sizeOfDuplicatedFiles failed", query);
        return 0;
    }
    if (!query.first())
        return 0;
    return query.value(0).toLongLong();
}

QString & NDatabase::addAND(QString & sql, bool *AND)
{
    Q_ASSERT(AND);
    if (*AND)
        sql += "AND ";
    else
    {
        sql += "WHERE ";
        *AND = true;
    }
    return sql;
}


QString NDatabase::stringToSortDirection(const QString & dir)
{
    if (dir.isEmpty() || (
            QString::compare(dir, "DESC", Qt::CaseInsensitive) != 0  &&
            QString::compare(dir, "ASC", Qt::CaseInsensitive) != 0)
        )
        return "ASC";

    return dir.toUpper();
}

QString NDatabase::stringToFileField(const QString & field)
{
    if (field.isEmpty() ||
        (field != "id" &&
         field != "fileName"&&
         field != "relativePath"&&
         field != "absoluteFilePath"&&
         field != "category_id"&&
         field != "hash"&&
         field != "deleted"&&
         field != "added"&&
         field != "size"&&
         field != "lastModified"&&

         field != "title"&&
         field != "album"&&
         field != "genre"&&
         field != "artist"&&
         field != "trackNumber"&&
         field != "year"&&
         field != "duration")
        // TODO: ADD OTHER FIELD
        )
        return "added";

    return field;
}

QString NDatabase::stringToUserField(const QString & field)
{
    if (field.isEmpty() ||
        (field != "id" &&
         field != "lastName"&&
         field != "firstName"&&
         field != "email"&&
         field != "password"&&
         field != "passwordRequested"&&
         field != "level"&&
         field != "registered")
        )
        return "lastName";

    return field;
}

bool NDatabase::getUserList(QScriptEngine & se, QScriptValue & dataArray,
                            const QStringList & searches, int start, int limit,
                            const QString & sort, const QString & dir)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT id, lastName, firstName, email, password, passwordRequested, level, registered FROM users ";

    bool selector = false;

    for (int i = 0; i < searches.count(); ++i){
        addAND(sql, &selector);
        sql += QString("(lastName LIKE :lastName%1 OR firstName LIKE :firstName%1 OR email LIKE :email%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY %1 %2 LIMIT :limit OFFSET :offset").
           arg(stringToUserField(sort)).
           arg(stringToSortDirection(dir));


    if (!query.prepare(sql))
    {
        debugLastQuery("userList prepare failed", query);
        return false;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":lastName%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":firstName%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":email%1").arg(i), QString("%%1%").arg(s));
    }

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("userList failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
    int fieldLastName = query.record().indexOf("lastName");
    int fieldFirstName = query.record().indexOf("firstName");
    int fieldEmail = query.record().indexOf("email");
    int fieldPassword = query.record().indexOf("password");
    int fieldpasswordRequested = query.record().indexOf("passwordRequested");
    int fieldLevel = query.record().indexOf("level");
    int fieldRegistered = query.record().indexOf("registered");

    int i = 0;
    while (query.next()) {
        QScriptValue svUser = se.newObject();
        dataArray.setProperty(i, svUser);
        i++;

        svUser.setProperty("id", query.value(fieldId).toInt());
        svUser.setProperty("lastName", query.value(fieldLastName).toString());
        svUser.setProperty("firstName", query.value(fieldFirstName).toString());
        svUser.setProperty("email", query.value(fieldEmail).toString());
        svUser.setProperty("password", query.value(fieldPassword).toString());
        svUser.setProperty("passwordRequested", query.value(fieldpasswordRequested).toBool());
        svUser.setProperty("level", query.value(fieldLevel).toString());
        svUser.setProperty("registered", query.value(fieldRegistered).toBool());

    }
    return true;
}

int NDatabase::getUserListCount(const QStringList & searches)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT count(*) FROM users ";

    bool selector = false;
    for (int i = 0; i < searches.count(); ++i){
        addAND(sql, &selector);
        sql += QString("(lastName LIKE :lastName%1 OR firstName LIKE :firstName%1 OR email LIKE :email%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        debugLastQuery("fileListCount prepare failed", query);
        return 0;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":lastName%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":firstName%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":email%1").arg(i), QString("%%1%").arg(s));
    }

    if (!query.exec())
    {
        debugLastQuery("fileListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}

void NDatabase::setUserLevel(const QString &)
{
    // TODO
}

bool NDatabase::registerUser(const QString & firstName, const QString & lastName,
                             const QString & email, const QString & password)
{
    // TODO
    if (firstName.isEmpty() || lastName.isEmpty() || email.isEmpty() || password.isEmpty())
        return false;

    QSqlQuery query(m_db);
    if (!query.prepare("INSERT INTO users (lastName, firstName, email, password) "\
                       "VALUES(:lastName, :firstName, :email, :password)"))
    {
        debugLastQuery("registerUser prepare failed", query);
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(password.toUtf8());
    hash.addData(NCONFIG.dbPwdHashKey());

    query.bindValue(":lastName", lastName);
    query.bindValue(":firstName", firstName);
    query.bindValue(":email", email);
    query.bindValue(":password", QString(hash.result().toHex()));
    if (!query.exec())
    {
        debugLastQuery("registerUser failed", query);
        return false;
    }
    return true;
}

const NStringMap NDatabase::getUser(const QString & email)
{	
    // TODO
    NStringMap user;
    if (email.isEmpty())
        return user;

    QSqlQuery query(m_db);
    QString sql = "SELECT lastName, firstName, email, password,"\
                  "passwordRequested level, registered " \
                  "FROM users " \
                  "WHERE email = :email";

    if (!query.prepare(sql))
    {
        debugLastQuery("getUser prepare failed", query);
        return user;
    }

    query.bindValue(":email", email);

    if (!query.exec())
    {
        debugLastQuery("getUser failed", query);
        return user;
    }

    int fieldLastName = query.record().indexOf("lastName");
    int fieldFirstName = query.record().indexOf("firstName");
    int fieldEmail = query.record().indexOf("email");
    int fieldPassword = query.record().indexOf("password");
    int fieldpasswordRequested = query.record().indexOf("passwordRequested");
    int fieldLevel = query.record().indexOf("level");
    int fieldRegistered = query.record().indexOf("registered");

    if (!query.first())
        return user;

    user["lastName"] = query.value(fieldLastName).toString();
    user["firstName"] = query.value(fieldFirstName).toString();
    user["email"] = query.value(fieldEmail).toString();
    user["password"] = query.value(fieldPassword).toString();
    user["passwordRequested"] = query.value(fieldpasswordRequested).toString();
    user["level"] = query.value(fieldLevel).toString();
    user["registered"] = query.value(fieldRegistered).toString();

    return user;
}

/*bool NDatabase::deleteUser(const QString & email)
{
	if (email.isEmpty())
		return false;

	QSqlQuery query(m_db);
	if (!query.prepare("DELETE FROM users WHERE email=:email"))
	{
		debugLastQuery("deleteUser prepare failed", query);
		return false;
	}
	query.bindValue(":email", email);
	if (!query.exec())
	{
		debugLastQuery("deleteUser failed", query);
		return false;
	} 
	return true;
}*/

bool NDatabase::deleteUser(const QString & id)
{
    // TODO
    if (id.isEmpty())
        return false;

    QSqlQuery query(m_db);
    if (!query.prepare("DELETE FROM users WHERE id=:id"))
    {
        debugLastQuery("deleteUser prepare failed", query);
        return false;
    }
    query.bindValue(":id", id);
    if (!query.exec())
    {
        debugLastQuery("deleteUser failed", query);
        return false;
    }
    return true;
}

bool NDatabase::requestUserPassord(const QString & email)
{
    // TODO
    if (email.isEmpty())
        return false;

    QSqlQuery query(m_db);
    if (!query.prepare("UPDATE users SET passwordRequested = 1 WHERE email=:email"))
    {
        debugLastQuery("requestUserPassord prepare failed", query);
        return false;
    }
    query.bindValue(":email", email);
    if (!query.exec())
    {
        debugLastQuery("requestUserPassord failed", query);
        return false;
    }
    return true;
}

bool NDatabase::updateMusicAlbumTable()
{
    beginTransaction();
    if (!setMusicAlbumDeleted())
    {
        abortTransaction();
        return false;
    }

    if (!populateMusicAlbum())
    {
        abortTransaction();
        return false;
    }

    if (!removeDeletedMusicAlbum())
    {
        abortTransaction();
        return false;
    }

    commitTransaction();
    return true;
}

bool NDatabase::populateMusicAlbum()
{
    QSqlQuery query(m_db);

    QString sql = "SELECT DISTINCT album "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND album IS NOT NULL "\
                  "AND files.hash <> '' ";

    if (!query.prepare(sql))
    {
        debugLastQuery("populateMusicAlbum prepare failed", query);
        return false;
    }

    if (!query.exec())
    {
        debugLastQuery("populateMusicAlbum failed", query);
        return false;
    }

    int fieldAlbum = query.record().indexOf("album");

    while (query.next()) {

        if (!insertMusicAlbum(query.value(fieldAlbum).toString()))
            return false;
    }

    return true;
}

bool NDatabase::insertMusicAlbum(const QString & albumName)
{
    QSqlQuery query(m_db);
    if (!query.prepare("INSERT INTO music_album (name) "\
                       "VALUES(:name)"))
    {
        debugLastQuery("insertMusicAlbum prepare failed", query);
        return false;
    }

    query.bindValue(":name", albumName);

    if (!query.exec())
    {
        // uncomment for DEBUG if needed
        //debugLastQuery("insertMusicAlbum failed", query);
        return setMusicAlbumDeleted(albumName, false);
    }

    return true;
}

bool NDatabase::setMusicAlbumDeleted(const QString albumName, bool deleted)
{
    QSqlQuery query(m_db);

    QString sql = "UPDATE music_album "\
                  "SET deleted=:deleted";

    if (!albumName.isEmpty())
        sql += " WHERE name=:name";

    if (!query.prepare(sql))
    {
        debugLastQuery("setMusicAlbumDeleted prepare failed", query);
        return false;
    }

    query.bindValue(":deleted", deleted ? 1 : 0);
    if (!albumName.isEmpty())
        query.bindValue(":name", albumName);

    if (!query.exec())
    {
        debugLastQuery("setMusicAlbumDeleted failed", query);
        return false;
    }
    return true;
}

bool NDatabase::removeDeletedMusicAlbum()
{
    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM music_album WHERE deleted=1"))
    {
        debugLastQuery("removeDeletedMusicAlbum failed", query);
        return false;
    }
    return true;
}

bool NDatabase::getMusicAlbumList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                                  const QStringList & searches, int start, int limit,
                                  const QString & dir, int year, const QString & genre,
                                  const QString & artist)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT music_album.name "\
                  "FROM music_album, metadata, files "\
                  "WHERE music_album.name = metadata.album "\
                  "AND files.";

    /*QString sql = "SELECT DISTINCT album "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND album IS NOT NULL "\
                  "AND files.hash <> '' ";*/

    /*  "select metadata.album, files.relativePath "\
        "from files,metadata "\
        "where files.metadata_id=metadata.id "\
        "AND album IS NOT NULL "\
        "AND files.hash <> '' "\
        "group by metadata.album";
    */

    /*  "SELECT album.name "\
        "FROM album, metadata "\
        "WHERE album.name = metadata.album "\
        "AND album IS NOT NULL "\
        "AND files.hash <> '' ";
    */


    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";
    if (!genre.isNull())
        sql += "AND metadata.genre = :genre ";
    if (!artist.isNull())
        sql += "AND metadata.artist = :artist ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY album %2 LIMIT :limit OFFSET :offset").
           arg(stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicAlbumList prepare failed", query);
        return false;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isNull())
        query.bindValue(":genre", genre);
    if (!artist.isNull())
        query.bindValue(":artist", artist);

    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("getMusicAlbumList failed", query);
        return false;
    }

    int fieldAlbum = query.record().indexOf("album");

    int i = 0;
    // We add 1 to result to manage "album-all"
    // "album-all" value is not returned if number of value <= 1
    if (start == 0 &&
        totalCount >= 2 &&
        dir.compare("DESC", Qt::CaseInsensitive))
    {
        QScriptValue svAlbum = se.newObject();
        dataArray.setProperty(i, svAlbum);
        i++;
        svAlbum.setProperty("album", QScriptValue("album-all"));
    }

    while (query.next()) {
        QScriptValue svAlbum = se.newObject();
        dataArray.setProperty(i, svAlbum);
        svAlbum.setProperty("album", query.value(fieldAlbum).toString());
        i++;
    }

    if (start + limit >= totalCount &&
        totalCount >= 2 &&
        dir.compare("ASC", Qt::CaseInsensitive))
    {
        QScriptValue svAlbum = se.newObject();
        dataArray.setProperty(i, svAlbum);
        i++;
        svAlbum.setProperty("album", QScriptValue("album-all"));
    }

    return true;
}

int NDatabase::getMusicAlbumListCount(const QStringList & searches, int year, const QString & genre,
                                      const QString & artist)
{
    NStringMapList list;
    QSqlQuery query(m_db);
    QString sql = "SELECT count(DISTINCT album) "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND album IS NOT NULL "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";
    if (!genre.isNull())
        sql += "AND metadata.genre = :genre ";
    if (!artist.isNull())
        sql += "AND metadata.artist = :artist ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicAlbumListCount prepare failed", query);
        return 0;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isNull())
        query.bindValue(":genre", genre);
    if (!artist.isNull())
        query.bindValue(":artist", artist);

    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }


    if (!query.exec())
    {
        debugLastQuery("getMusicAlbumListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;

    // We add 1 to result to manage "album-all"
    // "artist-all" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NDatabase::getMusicArtistList(QScriptEngine & se, QScriptValue & dataArray,
                                   int totalCount, const QStringList & searches, int start,
                                   int limit, const QString & dir, int year,
                                   const QString & genre)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT DISTINCT metadata.artist "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND metadata.artist IS NOT NULL "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";
    if (!genre.isNull())
        sql += "AND metadata.genre = :genre ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY metadata.artist %2 LIMIT :limit OFFSET :offset").
           arg(stringToSortDirection(dir));


    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicArtistList prepare failed", query);
        return false;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isNull())
        query.bindValue(":genre", genre);

    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("getMusicArtistList failed", query);
        return false;
    }

    int fieldArtist = query.record().indexOf("artist");
    int i = 0;
    // We add 1 to result to manage "artist-all"
    // "artist-all" value is not returned if number of value <= 1
    if (start == 0 &&
        totalCount >= 2 &&
        dir.compare("DESC", Qt::CaseInsensitive))
    {
        QScriptValue svArtist = se.newObject();
        dataArray.setProperty(i, svArtist);
        i++;
        svArtist.setProperty("artist", QScriptValue("artist-all"));
    }

    while (query.next()) {
        QScriptValue svArtist = se.newObject();
        dataArray.setProperty(i, svArtist);
        svArtist.setProperty("artist", query.value(fieldArtist).toString());
        i++;
    }

    if (start + limit >= totalCount &&
        totalCount >= 2 &&
        dir.compare("ASC", Qt::CaseInsensitive))
    {
        QScriptValue svArtist = se.newObject();
        dataArray.setProperty(i, svArtist);
        i++;
        svArtist.setProperty("artist", QScriptValue("artist-all"));
    }

    return true;
}

int NDatabase::getMusicArtistListCount(const QStringList & searches, int year, const QString & genre)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT count(DISTINCT  metadata.artist) "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND metadata.artist IS NOT NULL "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";
    if (!genre.isNull())
        sql += "AND metadata.genre = :genre ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicArtistListCount prepare failed", query);
        return 0;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isNull())
        query.bindValue(":genre", genre);

    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        debugLastQuery("getMusicArtistListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;

    // We add 1 to result to manage "artist-all"
    // "artist-all" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NDatabase::getMusicGenreList(QScriptEngine & se, QScriptValue & dataArray,
                                  int totalCount, const QStringList & searches, int start,
                                  int limit, const QString & dir, int year)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT DISTINCT genre "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND genre IS NOT NULL "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY genre %2 LIMIT :limit OFFSET :offset").
           arg(stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicGenreList prepare failed", query);
        return false;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);

    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("getMusicGenreList failed", query);
        return false;
    }

    int fieldGenre = query.record().indexOf("genre");

    int i = 0;
    // We add 1 to result to manage "genre-all"
    // "genre-all" value is not returned if number of value <= 1
    if (start == 0 &&
        totalCount >= 2 &&
        dir.compare("DESC", Qt::CaseInsensitive))
    {
        QScriptValue svGenre = se.newObject();
        dataArray.setProperty(i, svGenre);
        svGenre.setProperty("genre", QScriptValue("genre-all"));
        i++;
    }

    while (query.next()) {
        QScriptValue svGenre = se.newObject();
        dataArray.setProperty(i, svGenre);
        svGenre.setProperty("genre", query.value(fieldGenre).toString());
        i++;
    }

    if (start + limit >= totalCount &&
        totalCount >= 2 &&
        dir.compare("ASC", Qt::CaseInsensitive))
    {
        QScriptValue svGenre = se.newObject();
        dataArray.setProperty(i, svGenre);
        svGenre.setProperty("genre", QScriptValue("genre-all"));
        i++;
    }

    return true;
}

int NDatabase::getMusicGenreListCount(const QStringList & searches, int year)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT count(DISTINCT genre) "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND genre IS NOT NULL "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicGenreListCount prepare failed", query);
        return 0;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);

    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        debugLastQuery("getMusicGenreListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;

    // We add 1 to result to manage "genre-all"
    // "genre-all" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NDatabase::getMusicYearList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                                 const QStringList & searches, int start,
                                 int limit, const QString & dir)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT DISTINCT year "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND year IS NOT NULL "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.year = :year%1 ").arg(i);
        sql += QString("OR metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY year %2 LIMIT :limit OFFSET :offset").
           arg(stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicYearList prepare failed", query);
        return false;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));

    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":year%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("getMusicYearList failed", query);
        return false;
    }

    // Field
    int fieldYear = query.record().indexOf("year");

    int i = 0;
    // We add 1 to result to manage "genre-all"
    // "genre-all" value is not returned if number of value <= 1
    if (start == 0 &&
        totalCount >= 2 &&
        dir.compare("ASC", Qt::CaseInsensitive))
    {
        QScriptValue svYear = se.newObject();
        dataArray.setProperty(i, svYear);
        svYear.setProperty("year", -1);
        i++;
    }

    while (query.next()) {
        QScriptValue svYear = se.newObject();
        dataArray.setProperty(i, svYear);
        svYear.setProperty("year", query.value(fieldYear).toInt());
        i++;
    }


    if (totalCount >= 2 &&
        start + limit >= totalCount &&
        dir.compare("DESC", Qt::CaseInsensitive))
    {
        QScriptValue svYear = se.newObject();
        dataArray.setProperty(i, svYear);
        svYear.setProperty("year", -1);
    }

    return true;
}

int NDatabase::getMusicYearListCount(const QStringList & searches)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT count(DISTINCT year) "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND year IS NOT NULL "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.year = :year%1 ").arg(i);
        sql += QString("OR metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicYearListCount prepare failed", query);
        return 0;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    for (int i = 0; i < searches.count(); ++i){
        query.bindValue(QString(":year%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        debugLastQuery("getMusicYearListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;

    // We add 1 to result to manage "-1"
    // "-1" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NDatabase::getMusicTitleList(QScriptEngine & se, QScriptValue & dataArray,
                                  const QStringList & searches, const QString & album,
                                  const QString & artist, const QString & genre,
                                  int year, int start, int limit, const QString & sort,
                                  const QString & dir)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT files.id id, files.fileName, files.hash, files.size, "\
                  "metadata.artist, metadata.comment, metadata.year, metadata.album, "\
                  "metadata.title, metadata.genre, metadata.trackNumber, metadata.duration, "\
                  "metadata.copyright, metadata.hasID3Picture "\
                  "FROM files, metadata " \
                  "WHERE files.metadata_id = metadata.id "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";
    if (!genre.isNull())
        sql += "AND metadata.genre = :genre ";
    if (!artist.isNull())
        sql += "AND metadata.artist = :artist ";
    if (!album.isNull())
        sql += "AND metadata.album = :album ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY %1 %2 LIMIT :limit OFFSET :offset").
           arg(stringToFileField(sort)).
           arg(stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicTitleList prepare failed", query);
        return false;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isNull())
        query.bindValue(":genre", genre);
    if (!artist.isNull())
        query.bindValue(":artist", artist);
    if (!album.isNull())
        query.bindValue(":album", album);

    for (int i = 0; i < searches.count(); ++i){
        QString s = searches.at(i);
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit, start
    query.bindValue(":limit", limit);
    query.bindValue(":offset", start);

    //TMP  if (!query.exec())
    if (!query.exec())
    {
        debugLastQuery("getMusicTitleList failed", query);
        return false;
    } else {
        NLOGD("NDatabase", query.lastQuery());
    }

    // Files fields
    int fieldId = query.record().indexOf("id");
    int fieldFileName = query.record().indexOf("fileName");
    int fieldHash = query.record().indexOf("hash");
    int fieldSize = query.record().indexOf("size");

    int fieldArtist = query.record().indexOf("artist");
    int fieldComment = query.record().indexOf("comment");
    int fieldYear = query.record().indexOf("year");
    int fieldAlbum = query.record().indexOf("album");
    int fieldTitle = query.record().indexOf("title");
    int fieldGenre = query.record().indexOf("genre");
    int fieldTrackNumber = query.record().indexOf("trackNumber");
    int fieldDuration = query.record().indexOf("duration");
    int fieldCopyright = query.record().indexOf("copyright");
    int hasID3Picture = query.record().indexOf("hasID3Picture");


    int i = 0;
    while (query.next()) {
        QScriptValue svTitle = se.newObject();
        dataArray.setProperty(i, svTitle);
        i++;

        svTitle.setProperty("id", query.value(fieldId).toInt());
        svTitle.setProperty("fileName", query.value(fieldFileName).toString());
        svTitle.setProperty("hash", query.value(fieldHash).toString());
        svTitle.setProperty("size", query.value(fieldSize).toInt());

        // Metadata field
        svTitle.setProperty("artist", query.value(fieldArtist).toString());
        svTitle.setProperty("comment", query.value(fieldComment).toString());
        svTitle.setProperty("year", query.value(fieldYear).toInt());
        svTitle.setProperty("album", query.value(fieldAlbum).toString());
        svTitle.setProperty("title", query.value(fieldTitle).toString());
        svTitle.setProperty("genre", query.value(fieldGenre).toString());
        svTitle.setProperty("trackNumber", query.value(fieldTrackNumber).toInt());
        svTitle.setProperty("duration", query.value(fieldDuration).toInt());
        svTitle.setProperty("copyright", query.value(fieldCopyright).toString());
        svTitle.setProperty("hasID3Picture", query.value(hasID3Picture).toString());
    }

    return true;
}

int NDatabase::getMusicTitleListCount(const QStringList & searches, const QString & album,
                                      const QString & artist, const QString & genre,
                                      int year)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT COUNT(*) "\
                  "FROM files, metadata "\
                  "WHERE files.metadata_id = metadata.id "\
                  "AND files.hash <> '' ";

    sql += "AND files.category_id = :category_id ";
    if (year >= 0)
        sql += "AND metadata.year = :year ";
    if (!genre.isNull())
        sql += "AND metadata.genre = :genre ";
    if (!artist.isNull())
        sql += "AND metadata.artist = :artist ";
    if (!album.isNull())
        sql += "AND metadata.album = :album ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (metadata.genre LIKE :genre%1 ").arg(i);
        sql += QString("OR metadata.artist LIKE :artist%1 ").arg(i);
        sql += QString("OR metadata.album LIKE :album%1 ").arg(i);
        sql += QString("OR metadata.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        debugLastQuery("getMusicTitleListCount prepare failed", query);
        return 0;
    }

    query.bindValue(":category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));
    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isNull())
        query.bindValue(":genre", genre);
    if (!artist.isNull())
        query.bindValue(":artist", artist);
    if (!album.isNull())
        query.bindValue(":album", album);

    for (int i = 0; i < searches.count(); ++i){
        QString s = searches.at(i);
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        debugLastQuery("getMusicTitleListCount failed", query);
        return 0;
    }else {
        NLOGD("NDatabase", query.lastQuery());
    }

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}
