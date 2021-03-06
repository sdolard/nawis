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
#include "n_music_database.h"
#include "n_sqlite_error.h"
#include "n_tcp_server_auth_session.h"

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

    NMusicDatabase::instance(&m_db);
}

NDatabase::~NDatabase()
{
    abortTransaction();
    m_db.close();
}

void NDatabase::createCategoryTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS file_category (" \
            "id INTEGER NOT NULL," \
            "name TEXT NOT NULL," \
            "PRIMARY KEY (id, name)" \
            ")"))
        debugLastQuery("file_category table creation failed", query);

    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_file_category_name ON file_category(name)"))
        debugLastQuery("idx_file_category_name creation failed", query);


    // Default values
    if(!query.prepare("INSERT INTO file_category (id, name) VALUES(:id, :name)"))
    {
        debugLastQuery("createDefautValues prepare failed", query);
        return;
    }

    for(NFileCategory_n::FileCategory fc = NFileCategory_n::fcAll; fc <= NFileCategory_n::fcDocument; fc++)
    {
        query.bindValue(":id", NFileCategory_n::fileCategoryId(fc));
        query.bindValue(":name", NFileCategory_n::fileCategoryName(fc));

        if (!query.exec())
        {
            if (query.lastError().number() != SQLITE_CONSTRAINT)
                debugLastQuery("createCategoryTable: createDefautValues failed", query);
        }
    }
}

void NDatabase::createFileMetadataTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS file_metadata ("\
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"\
            "artist TEXT,"\
            "comment TEXT,"\
            "year INTEGER,"\
            "album TEXT,"\
            "title TEXT,"\
            "genre TEXT,"\
            "track_number INTEGER,"\
            "duration INTEGER,"\
            "has_id3_picture BOOLEAN DEFAULT 0 NOT NULL,"\
            "date_time_original TIMESTAMP,"\
            "copyright TEXT,"\
            "width INTEGER,"\
            "height INTEGER,"\
            "make TEXT,"\
            "model TEXT,"\
            "city TEXT,"\
            "province_state TEXT,"\
            "country TEXT,"\
            "latitude TEXT,"\
            "longitude TEXT,"\
            "altitude INTEGER"\
            ")"))
        debugLastQuery("file_metadata table creation failed", query);


    if (!query.exec(
            "CREATE TRIGGER IF NOT EXISTS delete_file_metadata " \
            "BEFORE DELETE ON file "\
            "FOR EACH ROW BEGIN "\
            "  DELETE FROM file_metadata WHERE id = OLD.fk_file_metadata_id; "\
            "END; "))
        debugLastQuery("file CREATE TRIGGER delete_file_metadata failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_file_metadata_album_index ON file_metadata(album)"))
        debugLastQuery("file_metadata CREATE INDEX idx_file_metadata_album_index failed", query);
}

void NDatabase::createFileTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS file (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fk_file_category_id INTEGER NOT NULL," \
            "fk_file_metadata_id INTEGER "\
            "  CONSTRAINT ct_fk_file_metadata_id REFERENCES file_metadata(id) ON DELETE CASCADE,"\
            "file_name TEXT NOT NULL," \
            "relative_path TEXT NOT NULL," \
            "absolute_file_path TEXT UNIQUE NOT NULL," \
            "hash TEXT,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL," \
            "added TIMESTAMP NOT NULL,"
            "size INTEGER NOT NULL,"
            "last_modified TIMESTAMP NOT NULL"
            ")"))
        debugLastQuery("file table creation failed", query);


    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_file_index "\
                    "ON file(absolute_file_path, last_modified)"))
        debugLastQuery("idx_file_index creation failed", query);


    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_file_hash "\
                    "ON file(hash)"))
        debugLastQuery("idx_file_hash creation failed", query);
}

void NDatabase::createDuplicatedFileTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS duplicated_file (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fk_file_category_id INTEGER NOT NULL," \
            "file_name TEXT NOT NULL," \
            "relative_path TEXT NOT NULL," \
            "absolute_file_path TEXT UNIQUE NOT NULL," \
            "hash TEXT NOT NULL,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL," \
            "added TIMESTAMP NOT NULL,"
            "size INTEGER NOT NULL,"
            "last_modified TIMESTAMP NOT NULL"
            ")"))
        debugLastQuery("duplicated_file table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_duplicated_file_index "\
                    "ON duplicated_file(hash, absolute_file_path)"))
        debugLastQuery("idx_duplicated_file_index creation failed", query);
}

void NDatabase::createUserTable()
{
    QSqlQuery query(m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS user (" \
            "  id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "  email TEXT NOT NULL," \
            "  password TEXT NOT NULL," \
            "  name TEXT NOT NULL," \
            "  enabled BOOLEAN DEFAULT 0 NOT NULL," \
            "  level TEXT," \
            "  preferences TEXT" \
            ")"))
        debugLastQuery("user table creation failed", query);
}


void NDatabase::createTables()
{
    // Enable foreign key
    enableFk();

    // Order is important!
    createCategoryTable();
    createFileTable();
    createFileMetadataTable();
    createDuplicatedFileTable();
    createUserTable();
}

void NDatabase::enableFk()
{
    QSqlQuery query(m_db);
    if (!query.exec("PRAGMA foreign_keys = ON"))
        debugLastQuery("enableFk failed", query);
}

void NDatabase::debugLastQuery(const QString & msg, const QSqlQuery & query)
{
    logDebugDirect("NDatabase", "Data base error:");
    logDebugDirect("NDatabase", QString("  %1").arg(msg));
    logDebugDirect("NDatabase", QString("  %1").arg(query.lastError().number()));
    logDebugDirect("NDatabase", QString("  %1").arg(query.lastError().text()));
    logDebugDirect("NDatabase", QString("  %1").arg(query.lastQuery()));
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
    if (!query.exec("UPDATE file SET deleted = 1"))
    {
        debugLastQuery("setFilesAsDeleted failed", query);
        return false;
    }
    return true;
}

bool NDatabase::setDuplicatedFilesAsDeleted()
{
    QSqlQuery query(m_db);
    if (!query.exec("UPDATE duplicated_file SET deleted = 1"))
    {
        debugLastQuery("setDuplicatedFilesAsDeleted failed", query);
        return false;
    }
    return true;
}

bool NDatabase::setFileAsNotDeleted(const QString & absoluteFilePath, const QDateTime & lastModified)
{
    QSqlQuery query(m_db);
    if(!query.prepare("UPDATE file SET deleted = 0 "\
                      "WHERE absolute_file_path = :absolute_file_path "\
                      "AND last_modified = :last_modified")) // same file, no update hash needed
    {
        debugLastQuery("setFileAsNotDeleted prepare failed", query);
        return false;
    }

    query.bindValue(":absolute_file_path", absoluteFilePath);
    query.bindValue(":last_modified", lastModified.toString(SQLITE_DATETIME));
    if (!query.exec())
    {
        debugLastQuery("setFileAsNotDeleted failed", query);
        return false;
    }
    return query.numRowsAffected() == 1;
}

// This function is OK
bool NDatabase::setDuplicatedFileAsNotDeleted(const QFileInfo & fi)
{
    if (!fi.isFile())
        return false;

    QSqlQuery query(m_db);
    if (!query.prepare("UPDATE duplicated_file SET deleted = 0 "\
                       "WHERE absolute_file_path = :absolute_file_path "\
                       "AND last_modified = :last_modified")) // It has to be the same file, during all scan update
    {
        debugLastQuery("setDuplicatedFileAsNotDeleted prepare failed", query);
        return false;
    }
    query.bindValue(":absolute_file_path", fi.absoluteFilePath());
    query.bindValue(":last_modified", fi.lastModified().toString(SQLITE_DATETIME));
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
    if (!query.exec("DELETE FROM file WHERE deleted = 1"))
    {
        debugLastQuery("removeDeletedFiles failed", query);
        return false;
    }
    return true;
}

bool NDatabase::removeDeletedDuplicatedFiles()
{
    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM duplicated_file WHERE deleted = 1"))
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
    if (!query.prepare("INSERT INTO file (file_name, relative_path, "\
                       "absolute_file_path, fk_file_category_id, added, size, last_modified) "\
                       "VALUES(:file_name, :relative_path, :absolute_file_path, :fk_file_category_id, "\
                       ":added, :size, :last_modified)"))
    {
        debugLastQuery("addFile prepare failed", query);
        return false;
    }

    query.bindValue(":file_name", fi.fileName());
    query.bindValue(":relative_path", fi.absolutePath().remove(rootPath));
    query.bindValue(":absolute_file_path", absoluteFilePath);
    query.bindValue(":fk_file_category_id", suffix.category());
    query.bindValue(":added", QDateTime::currentDateTime().toString(SQLITE_DATETIME));
    query.bindValue(":size", fi.size());
    query.bindValue(":last_modified", lastModified.toString(SQLITE_DATETIME));

    if (!query.exec())
    {
        if (query.lastError().number() == SQLITE_CONSTRAINT){
            if (setFileAsNotDeleted(absoluteFilePath, lastModified))
                return true;
            query.clear();
            if (!query.prepare("DELETE FROM file "\
                               "WHERE absolute_file_path = :absolute_file_path "))
                debugLastQuery("addFile prepare failed (1)", query);

            query.bindValue(":absolute_file_path", absoluteFilePath);
            if (!query.exec())
                debugLastQuery("addFile prepare failed (2)", query);

            return addFile(fi, suffix, rootPath);
        }
        debugLastQuery("addFile failed", query);
        return false;
    }
    return true;
}

// Algo is ok but it's unreadable (optimized) :)
const QFileInfo NDatabase::getFileToHash()
{
    QSqlQuery query(m_db);
    while (true) {
        /**
        * Step 1: we are looking for a file to hash
        */
        if (!query.exec("SELECT absolute_file_path "\
                        "FROM file "\
                        "WHERE hash IS NULL "\
                        "AND deleted = 0 "\
                        "LIMIT 1 "))
        {
            debugLastQuery("getFileToHash failed (1)", query);
            return QFileInfo();
        }

        if (!query.first())
            return QFileInfo(); // no more file to hash

        QFileInfo fiToHash(query.value(0).toString()); // Here is the file to hash
        query.clear();

        /**
        * Step 2
        * Is this an already referenced duplicated file ?
        * Reference is updated when hash is set for a file
        * So this step already used after the FIRST FULL files scan
        */
        if (!query.prepare("SELECT file.absolute_file_path "\
                           "FROM file, duplicated_file "\
                           "WHERE file.deleted = 0 "\
                           "AND duplicated_file.absolute_file_path = :absolute_file_path "\
                           "AND file.hash = duplicated_file.hash"))
        {
            debugLastQuery("getFileToHash failed (2)", query);
            return QFileInfo();
        }
        query.bindValue(":absolute_file_path", fiToHash.absoluteFilePath());
        if (!query.exec())
        {
            debugLastQuery("getFileToHash failed (3)", query);
            return QFileInfo();
        }
        if (!query.first()){
            /**
             * Step 3: there is no duplicated file reference
             * We check if new file to hash still exists
             */
            if (fiToHash.exists())
                return fiToHash;
        } else {
            /**
             * Step 3 ': there is no duplicated file reference
             */
            QFileInfo dupFi(query.value(0).toString());
            if (!dupFi.exists()){// Duplicated file no more not exists
                if (fiToHash.exists())
                    return fiToHash;
            } else
                // Update deleted status of duplicated file reference
                setDuplicatedFileAsNotDeleted(fiToHash);
        }

        /**
         * Step 4: all test has been done: we can delete the file to hash from table
         * and looking for another file to hash until there is no more file to hash
         */
        query.clear();
        if (!query.prepare("DELETE FROM file "\
                           "WHERE absolute_file_path = :absolute_file_path "))
        {
            debugLastQuery("getFileToHash failed (4)", query);
        }
        query.bindValue(":absolute_file_path", fiToHash.absoluteFilePath());
        if (!query.exec())
        {
            debugLastQuery("getFileToHash failed (5)", query);
        }
    }
}

bool NDatabase::getFileWithNoMetadata(QString & absoluteFilePath, int & fileId)
{
    absoluteFilePath.clear();
    fileId = 0;

    QSqlQuery query(m_db);

    QString sql = "SELECT id, absolute_file_path "\
                  "FROM file "\
                  "WHERE fk_file_metadata_id IS NULL "\
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

    if (!query.prepare("INSERT INTO file_metadata (artist, comment, year, album, "\
                       "title, genre, track_number, duration,  date_time_original,"\
                       "copyright, width, height, make, model, latitude,"\
                       "longitude, altitude, city, province_state, country, has_id3_picture) " \
                       "VALUES(:artist, :comment, :year, :album, "\
                       ":title, :genre, :track_number, :duration,  :date_time_original,"\
                       ":copyright, :width, :height, :make, :model, :latitude,"\
                       ":longitude, :altitude, :city, :province_state, :country, :has_id3_picture)"))
    {
        debugLastQuery("setMetadata prepare failed", query);
        return false;
    }

    query.bindValue(":artist", metadata.artist());
    query.bindValue(":comment", metadata.comment());
    query.bindValue(":year", metadata.year());
    query.bindValue(":album", metadata.album());
    query.bindValue(":title", metadata.title());
    query.bindValue(":genre", metadata.genre());
    query.bindValue(":track_number", metadata.trackNumber());
    query.bindValue(":duration", metadata.duration());
    query.bindValue(":has_id3_picture", metadata.hasID3Picture());
    query.bindValue(":date_time_original", metadata.dateTimeOriginal().toString(SQLITE_DATETIME));
    query.bindValue(":copyright", metadata.copyright());
    query.bindValue(":width", metadata.width());
    query.bindValue(":height", metadata.height());
    query.bindValue(":make", metadata.make());
    query.bindValue(":model", metadata.model());
    query.bindValue(":latitude", metadata.latitude());
    query.bindValue(":longitude", metadata.longitude());
    query.bindValue(":altitude", metadata.altitude());
    query.bindValue(":city", metadata.city());
    query.bindValue(":province_state", metadata.provinceState());
    query.bindValue(":country", metadata.country());
    if (!query.exec())
    {
        debugLastQuery("setMetadata failed (0)", query);
        return false;
    }
    int lastInsertId = query.lastInsertId().toInt();
    query.clear();
    if (!query.prepare("UPDATE file SET fk_file_metadata_id=:file_metadata_id WHERE id=:id"))
    {
        debugLastQuery("setMetadata prepare failed (1)", query);
        return false;
    }
    query.bindValue(":file_metadata_id", lastInsertId);
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
    // TODO: LEFT OUTER JOIN file_metadata > if file metadata is empty, a record is not necessary(db will be more light)
    QSqlQuery query(m_db);
    QString sql = "SELECT file.id id, file.file_name, file.relative_path, file.hash, "\
                  "file.added, file.size, file.fk_file_category_id, file.last_modified, " \
                  "file_metadata.artist, file_metadata.comment, file_metadata.year, file_metadata.album, "\
                  "file_metadata.title, file_metadata.genre, file_metadata.track_number, file_metadata.duration, "\
                  "file_metadata.date_time_original, file_metadata.copyright, file_metadata.width, file_metadata.height, "\
                  "file_metadata.make, file_metadata.model, file_metadata.latitude, file_metadata.longitude, file_metadata.altitude, "\
                  "file_metadata.city, file_metadata.province_state, file_metadata.country, file_metadata.has_id3_picture "\
                  "FROM file, file_metadata " \
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND file.hash <> '' ";

    if (fc != NFileCategory_n::fcAll)
        sql += "AND file.fk_file_category_id = :fk_file_category_id ";


    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (file.relative_path LIKE :relative_path%1 "\
                       "OR file.file_name LIKE :file_name%1 "
                       "OR file_metadata.artist LIKE :artist%1 "\
                       "OR file_metadata.comment LIKE :comment%1 "\
                       "OR file_metadata.album LIKE :album%1 "\
                       "OR file_metadata.title LIKE :title%1 "\
                       "OR file_metadata.genre LIKE :genre%1 "\
                       "OR file_metadata.make LIKE :make%1 "\
                       "OR file_metadata.city LIKE :city%1 "\
                       "OR file_metadata.province_state LIKE :province_state%1 "\
                       "OR file_metadata.country LIKE :country%1 "\
                       "OR file_metadata.model LIKE :model%1) ").arg(i);

    // Sort and limit
    sql += QString("ORDER BY %1 %2 ").
           arg(jsFileStringToDBFileField(sort)).
           arg(stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";

    if (!query.prepare(sql))
    {
        debugLastQuery("fileList prepare failed", query);
        return false;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":relative_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":file_name%1").arg(i), QString("%%1%").arg(s));

        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":comment%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(s));

        query.bindValue(QString(":make%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":model%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":city%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":province_state%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":country%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":fk_file_category_id", (NFileCategory_n::fileCategoryId(fc)));

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    //TMP  if (!query.exec())
    if (!query.exec())
    {
        debugLastQuery("fileList failed", query);
        return false;
    }/* else {
        logDebug("NDatabase", QString("  %1").arg(query.lastQuery()));
    }*/

    // Files fields
    int fieldId = query.record().indexOf("id");
    int fieldFileName = query.record().indexOf("file_name");
    int fieldRelativePath = query.record().indexOf("relative_path");
    int fieldCategoryId = query.record().indexOf("fk_file_category_id");
    int fieldHash = query.record().indexOf("hash");
    int fieldAdded = query.record().indexOf("added");
    int fieldSize = query.record().indexOf("size");
    int fieldLastModified = query.record().indexOf("last_modified");

    int fieldArtist = query.record().indexOf("artist");
    int fieldComment = query.record().indexOf("comment");
    int fieldYear = query.record().indexOf("year");
    int fieldAlbum = query.record().indexOf("album");
    int fieldTitle = query.record().indexOf("title");
    int fieldGenre = query.record().indexOf("genre");
    int fieldTrackNumber = query.record().indexOf("track_number");
    int fieldDuration = query.record().indexOf("duration");
    int hasID3Picture = query.record().indexOf("has_id3_picture");

    int fieldDateTimeOriginal = query.record().indexOf("date_time_original");
    int fieldCopyright = query.record().indexOf("copyright");
    int fieldWidth = query.record().indexOf("width");
    int fieldHeight = query.record().indexOf("height");
    int fieldMake = query.record().indexOf("make");
    int fieldModel = query.record().indexOf("model");
    int fieldLongitude = query.record().indexOf("longitude");
    int fieldLatitude = query.record().indexOf("latitude");
    int fieldAltitude = query.record().indexOf("altitude");

    int fieldCity = query.record().indexOf("city");
    int fieldProvinceState = query.record().indexOf("province_state");
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
        svfile.setProperty("fileCategory", NFileCategory_n::fileCategoryName(category));
        svfile.setProperty("hash", query.value(fieldHash).toString());
        svfile.setProperty("added", query.value(fieldAdded).toString());
        svfile.setProperty("size", query.value(fieldSize).toString());//TODO: Should be toULongLong
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
            // TODO: copyright only on picture ?
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
                  "FROM file, file_metadata "\
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND file.hash <> '' ";

    if (fc != NFileCategory_n::fcAll)
        sql += "AND file.fk_file_category_id = :fk_file_category_id ";

    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (file.relative_path LIKE :relative_path%1 "\
                       "OR file.file_name LIKE :file_name%1 "
                       "OR file_metadata.artist LIKE :artist%1 "\
                       "OR file_metadata.comment LIKE :comment%1 "\
                       "OR file_metadata.album LIKE :album%1 "\
                       "OR file_metadata.title LIKE :title%1 "\
                       "OR file_metadata.genre LIKE :genre%1 "\
                       "OR file_metadata.make LIKE :make%1 "\
                       "OR file_metadata.city LIKE :city%1 "\
                       "OR file_metadata.province_state LIKE :province_state%1 "\
                       "OR file_metadata.country LIKE :country%1 "\
                       "OR file_metadata.model LIKE :model%1) ").arg(i);


    if (!query.prepare(sql))
    {
        debugLastQuery("fileListCount prepare failed", query);
        return 0;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":relative_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":file_name%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":comment%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":make%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":model%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":city%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":province_state%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":country%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":fk_file_category_id", (NFileCategory_n::fileCategoryId(fc)));

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
    QString sql = "SELECT duplicated_file.id, duplicated_file.file_name, "\
                  "duplicated_file.relative_path, duplicated_file.absolute_file_path, "\
                  "duplicated_file.hash, duplicated_file.added, duplicated_file.size, "\
                  "duplicated_file.fk_file_category_id, "\
                  "file.absolute_file_path original_absolute_file_path, file.id original_file_id "\
                  "FROM duplicated_file "\
                  "  INNER JOIN file ON duplicated_file.hash = file.hash "\
                  "WHERE file.hash <> '' ";

    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (duplicated_file.relative_path LIKE :duplicated_relative_path%1  OR "\
                       "duplicated_file.absolute_file_path LIKE :duplicated_absolute_file_path%1  OR "\
                       "duplicated_file.file_name LIKE :duplicated_file_name%1  OR "\
                       "file.relative_path LIKE :relative_path%1  OR "\
                       "file.file_name LIKE :file_name%1) ").arg(i);

    if (fc != NFileCategory_n::fcAll)
        sql += "AND duplicated_file.fk_file_category_id = :fk_file_category_id ";

    // Sort and limit
    sql += QString("ORDER BY duplicated_file.%1 %2 ").
           arg(jsFileStringToDBFileField(sort)).
           arg(stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";

    if (!query.prepare(sql))
    {
        debugLastQuery("duplicatedFileList prepare failed", query);
        return false;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":duplicated_relative_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_absolute_file_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_file_name%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":relative_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":file_name%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":fk_file_category_id", (NFileCategory_n::fileCategoryId(fc)));

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("duplicatedFileList failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
    int fieldFileName = query.record().indexOf("file_name");
    int fieldRelativePath = query.record().indexOf("relative_path");
    int fieldAbsoluteFilePath = query.record().indexOf("absolute_file_path");
    int fieldCategoryId = query.record().indexOf("fk_file_category_id");
    int fieldHash = query.record().indexOf("hash");
    int fieldAdded = query.record().indexOf("added");
    int fieldSize = query.record().indexOf("size");
    int fieldOriginalAbsoluteFilePath = query.record().indexOf("original_absolute_file_path");
    int fieldOriginalFileId = query.record().indexOf("original_file_id");

    int i = 0;
    while (query.next()) {
        QScriptValue svfile = se.newObject();
        dataArray.setProperty(i, svfile);

        // Files field
        svfile.setProperty("id", QString(query.value(fieldId).toString() + query.value(fieldOriginalFileId).toString()).toInt()); // Cos it can have many duplicated files of same file
        svfile.setProperty("fileId", query.value(fieldId).toInt());
        svfile.setProperty("originalFileId", query.value(fieldOriginalFileId).toInt());
        svfile.setProperty("fileName", query.value(fieldFileName).toString());
        svfile.setProperty("relativePath", query.value(fieldRelativePath).toString());
        svfile.setProperty("absoluteFilePath", query.value(fieldAbsoluteFilePath).toString());
        NFileCategory_n::FileCategory fc = NFileCategory_n::toFileCategory(query.value(fieldCategoryId).toInt());
        svfile.setProperty("fileCategory", NFileCategory_n::fileCategoryName(fc));
        svfile.setProperty("hash", query.value(fieldHash).toString());
        svfile.setProperty("added", query.value(fieldAdded).toString());
        svfile.setProperty("size", query.value(fieldSize).toString());//TODO: Should be toULongLong
        svfile.setProperty("originalAbsoluteFilePath", query.value(fieldOriginalAbsoluteFilePath).toString());

        i++;
    }
    return true;
}


int NDatabase::getDuplicatedFileListCount(const QStringList & searches, NFileCategory_n::FileCategory fc)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT COUNT(duplicated_file.hash) " \
                  "FROM duplicated_file "\
                  "  INNER JOIN file ON duplicated_file.hash = file.hash "\
                  "WHERE file.hash <> '' ";

    for (int i = 0; i < searches.count(); ++i)
        sql += QString("AND (duplicated_file.relative_path LIKE :duplicated_relative_path%1  OR "\
                       "duplicated_file.absolute_file_path LIKE :duplicated_absolute_file_path%1  OR "\
                       "duplicated_file.file_name LIKE :duplicated_file_name%1  OR "\
                       "file.relative_path LIKE :relative_path%1  OR "\
                       "file.file_name LIKE :file_name%1) ").arg(i);

    if (fc != NFileCategory_n::fcAll)
        sql += QString("AND duplicated_file.fk_file_category_id = :fk_file_category_id ");

    if (!query.prepare(sql))
    {
        debugLastQuery("duplicatedFileListCount prepare failed", query);
        return 0;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":duplicated_relative_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_absolute_file_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":duplicated_file_name%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":relative_path%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":file_name%1").arg(i), QString("%%1%").arg(s));
    }

    if (fc != NFileCategory_n::fcAll)
        query.bindValue(":fk_file_category_id", (NFileCategory_n::fileCategoryId(fc)));


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
    if (!query.prepare("SELECT absolute_file_path, hash, added, size, fk_file_category_id " \
                       "FROM file "\
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

// Called in setFileHash: this function is ok
bool NDatabase::isDuplicatedFile(const QString & hash, const QFileInfo & newFi)
{
    /**
      * Step 1: is this hash already exists in file table?
      */
    QSqlQuery query(m_db);
    if (!query.prepare("SELECT file_name, relative_path, absolute_file_path "\
                       "FROM file "\
                       "WHERE hash = :hash"))
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

    /**
      * Step 2: We insert file in duplicated table or update delete flag if it exists
      */
    QString relativeFilePath = QDir::toNativeSeparators(QString("%1/%2").
                                                        arg(query.value(1).toString()).
                                                        arg(query.value(0).toString()));
    QString absoluteFilePath = query.value(2).toString();
    QString rootPath = absoluteFilePath.remove(relativeFilePath);

    query.clear();
    // we add it in duplicated file table
    if(!query.prepare("INSERT INTO duplicated_file (file_name, relative_path, "\
                      "absolute_file_path, fk_file_category_id, added, size, last_modified, hash) "\
                      "VALUES(:file_name, :relative_path, :absolute_file_path, :fk_file_category_id, "\
                      ":added, :size, :last_modified, :hash)"))
    {
        debugLastQuery("isDuplicatedFile prepare failed (2)", query);
        return false;
    }

    query.bindValue(":file_name", newFi.fileName());
    query.bindValue(":relative_path", newFi.absolutePath().remove(rootPath));
    query.bindValue(":absolute_file_path", newFi.absoluteFilePath());
    query.bindValue(":fk_file_category_id",
                    NFileCategory_n::fileCategoryId(getConfig().fileSuffixes().category(newFi).category()));
    query.bindValue(":added", QDateTime::currentDateTime().toString(SQLITE_DATETIME));
    query.bindValue(":size", newFi.size());
    query.bindValue(":last_modified", newFi.lastModified().toString(SQLITE_DATETIME));
    query.bindValue(":hash", hash);

    if (!query.exec())
    {
        if (query.lastError().number() == SQLITE_CONSTRAINT)
            setDuplicatedFileAsNotDeleted(newFi);
        else
            debugLastQuery("isDuplicatedFile failed (2)", query); //If file alreay exist, insert failed, it's normal.
    }
    query.clear();

    /**
      * Step 3: We delete file, cos it now is in duplicated file table
      */
    if (!query.prepare("DELETE FROM file WHERE absolute_file_path = :absolute_file_path"))
    {
        debugLastQuery("isDuplicatedFile prepare failed (3)", query);
        return false;
    }
    query.bindValue(":absolute_file_path", newFi.absoluteFilePath());
    if (!query.exec())
    {
        debugLastQuery("isDuplicatedFile failed (3)", query);
        return false;
    }
    return true;
}

// TODO: review this: related to addFile and getFileHash
void NDatabase::setFileHash(const QFileInfo & fi, const QString & hash)
{
    Q_ASSERT(fi.exists());
    Q_ASSERT(!hash.isEmpty());

    if (!fi.isFile())
        return;
    QString addedDateTime;
    QSqlQuery query(m_db);

    /**
      * Step 1: we are looking for a previous file with same hash
      * in order to not update added dateTime
      */
    if (!query.prepare("SELECT added FROM file WHERE hash = :hash"))
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

        /**
          * Step 1.1: Looking for file exists yet, doubled file
          */
        if (isDuplicatedFile(hash, fi))
            return;

        // Step 1.2 is not valid!
        /**
          * Step 1.2: we delete previous instance of this hash (we've got added datetime)
          */
        /*if (!query.prepare("DELETE FROM file WHERE hash = :hash"))
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
        query.clear();*/

        /**
          * Step 1.3: we update previous instance of this hash
          */
        if (!query.prepare("UPDATE file SET hash = :hash, added = :added "\
                           "WHERE absolute_file_path = :absolute_file_path "\
                           "AND last_modified = :last_modified"))

        {
            debugLastQuery("setFileHash prepare failed (3)", query);
            return;
        }
        query.bindValue(":hash", hash);
        query.bindValue(":added", addedDateTime);
        query.bindValue(":last_modified", fi.lastModified().toString(SQLITE_DATETIME));
        query.bindValue(":absolute_file_path", fi.absoluteFilePath());
        if (!query.exec())
        {
            debugLastQuery("setFileHash failed (3)", query);
            return;
        }

        /**
          * Step 1.4: File has changed or is changing since db insert to hash process,
          * we delete it, we can not do anything else
          */
        if(query.numRowsAffected() == 0)
        {
            if (!query.prepare("DELETE FROM file WHERE absolute_file_path = :absolute_file_path"))
            {
                debugLastQuery("setFileHash prepare failed (3.1)", query);
                return;
            }
            query.bindValue(":absolute_file_path", fi.absoluteFilePath());
            if (!query.exec())
            {
                debugLastQuery("setFileHash failed (3.1)", query);
                return;
            }
        }
        return;
    }

    /**
      * Step 2: file has not been updated
      */
    query.clear();
    if (!query.prepare("UPDATE file SET hash = :hash "\
                       "WHERE absolute_file_path = :absolute_file_path "\
                       "AND last_modified = :last_modified"))
    {
        debugLastQuery("setFileHash prepare failed (4)", query);
        return;
    }
    query.bindValue(":hash", hash);
    query.bindValue(":last_modified", fi.lastModified().toString(SQLITE_DATETIME));
    query.bindValue(":absolute_file_path", fi.absoluteFilePath());
    if (!query.exec())
    {
        debugLastQuery("setFileHash failed (4)", query);
        return;
    }

    /**
      * Step 3:File has changed or is changing since db insert to hash process,
      * we delete it, we can not do anything else
      */
    if(query.numRowsAffected() == 0)
    {
        if (!query.prepare("DELETE FROM file WHERE absolute_file_path = :absolute_file_path"))
        {
            debugLastQuery("setFileHash prepare failed (5)", query);
            return;
        }
        query.bindValue(":absolute_file_path", fi.absoluteFilePath());
        if (!query.exec())
        {
            debugLastQuery("setFileHash failed (5)", query);
            return;
        }
    }
}

qint64 NDatabase::sharedSize()
{
    QSqlQuery query(m_db);
    if (!query.exec("SELECT sum(size) FROM file"))
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
    if (!query.exec("SELECT sum(size) FROM duplicated_file"))
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

QString NDatabase::jsFileStringToDBFileField(const QString & jsString)
{
    if (jsString.isEmpty())
        return "added";

    if (jsString == "id" ||
        jsString == "hash"||
        jsString == "deleted"||
        jsString == "added"||
        jsString == "size"||
        jsString == "title"||
        jsString == "album"||
        jsString == "genre"||
        jsString == "artist"||
        jsString == "year"||
        jsString == "duration")
        return jsString;

    if(jsString == "fileName")
        return "file_name";

    if(jsString == "relativePath")
        return "relative_path";

    if(jsString == "fileName")
        return "absolute_file_path";

    if(jsString == "fileCategory")
        return "fk_file_category_id";

    if(jsString == "lastModified")
        return "last_modified";

    if(jsString == "trackNumber")
        return "track_number";

    if(jsString == "dateTimeOriginal")
        return "date_time_original";

    if(jsString == "hasID3Picture")
        return "has_id3_picture";

    if(jsString == "absoluteFilePath")
        return "absolute_file_path";

    if(jsString == "originalAbsoluteFilePath")
        return "absolute_file_path";

    Q_ASSERT_X(false, "NDatabase::jsFileStringToDBFileField", qPrintable(QString("%1 is not mapped" ).arg(jsString)));
    return "added";
}

QString NDatabase::stringToUserField(const QString & field)
{
    if (field.isEmpty() ||
        (field != "id" &&
         field != "email"&&
         field != "level"&&
         field != "name")
        )
        return "email";

    return field;
}

bool NDatabase::getUserList(QScriptEngine & se, QScriptValue & dataArray,
                            const QStringList & searches, int start, int limit,
                            const QString & sort, const QString & dir)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT * FROM user ";

    bool selector = false;
    for (int i = 0; i < searches.count(); ++i){
        addAND(sql, &selector);
        sql += QString("(name LIKE :name%1 OR email LIKE :email%1 OR level LIKE :level%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY %1 %2 ").
           arg(stringToUserField(sort)).
           arg(stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";

    if (!query.prepare(sql))
    {
        debugLastQuery("getUserList prepare failed", query);
        return false;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":email%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":name%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":level%1").arg(i), QString("%%1%").arg(s));
    }

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    if (!query.exec())
    {
        debugLastQuery("getUserList failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
    int fieldEmail = query.record().indexOf("email");
    int fieldName = query.record().indexOf("name");
    int fieldPreferences = query.record().indexOf("preferences");
    int fieldEnabled = query.record().indexOf("enabled");
    int fieldLevel = query.record().indexOf("level");

    int i = 0;
    while (query.next()) {
        QScriptValue svUser = se.newObject();
        dataArray.setProperty(i, svUser);
        i++;
        svUser.setProperty("id", query.value(fieldId).toInt());
        svUser.setProperty("email", query.value(fieldEmail).toString());
        svUser.setProperty("name", query.value(fieldName).toString());
        svUser.setProperty("preferences", query.value(fieldPreferences).toString());
        svUser.setProperty("enabled", query.value(fieldEnabled).toBool());
        svUser.setProperty("level",  NTcpServerAuthSession::normalizeLevels(query.value(fieldLevel).toString()));
    }
    return true;
}

int NDatabase::getUserListCount(const QStringList & searches)
{
    QSqlQuery query(m_db);
    QString sql = "SELECT count(id) FROM user ";

    bool selector = false;
    for (int i = 0; i < searches.count(); ++i){
        addAND(sql, &selector);
        sql += QString("(name LIKE :name%1 OR email LIKE :email%1 OR level LIKE :level%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        debugLastQuery("getUserListCount prepare failed", query);
        return 0;
    }

    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":email%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":name%1").arg(i), QString("%%1%").arg(s));
        query.bindValue(QString(":level%1").arg(i), QString("%%1%").arg(s));
    }

    if (!query.exec())
    {
        debugLastQuery("getUserListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}


int NDatabase::addUser(const QString & name, const QString & email, const QString & password,
                       const QString & level, const QString & preferences, bool enabled)
{
    /*
    "  email TEXT NOT NULL,"
    "  password TEXT NOT NULL,"
    "  name TEXT NOT NULL,"
    */
    if (email.isEmpty() || password.isEmpty() || name.isEmpty()) {
        return DB_USER_ERROR_INVALID_PARAMS;
    }

    QSqlQuery query(m_db);
    if (!query.prepare("INSERT INTO user (email, name, password, level, preferences, enabled) "\
                       "VALUES(:email, :name, :password, :level, :preferences, :enabled)"))
    {
        debugLastQuery("addUser prepare failed", query);
        return DB_USER_ERROR_QUERY;
    }

    query.bindValue(":name", name);
    query.bindValue(":email", email);
    query.bindValue(":password", password);
    query.bindValue(":preferences", preferences);
    query.bindValue(":enabled", enabled);
    query.bindValue(":level",  NTcpServerAuthSession::normalizeLevels(level));
    if (!query.exec())
    {
        debugLastQuery("addUser failed", query);
        return DB_USER_ERROR_QUERY;
    }
    return query.lastInsertId().toInt();
}

bool NDatabase::updateUser(int id, const QString & email, const QString & password,
                           const QString & name, const QString & preferences,
                           bool enabled, const QString & level)
{
    // TODO
    if (name.isEmpty() || email.isEmpty() || password.isEmpty())
        return false;

    QSqlQuery query(m_db);
    if (!query.prepare("UPDATE user "
                       "SET email=:email,"\
                       "password=:password,"\
                       "name=:name,"\
                       "preferences=:preferences,"\
                       "enabled=:enabled, "\
                       "level=:level "\
                       "WHERE id=:id"))
    {
        debugLastQuery("updateUser prepare failed", query);
        return false;
    }

    query.bindValue(":id", id);
    query.bindValue(":email", email);
    query.bindValue(":password", password);
    query.bindValue(":name", name);
    query.bindValue(":preferences", preferences);
    query.bindValue(":enabled", enabled);
    query.bindValue(":level",  NTcpServerAuthSession::normalizeLevels(level));

    if (!query.exec())
    {
        debugLastQuery("updateUser failed", query);
        return false;
    }
    return true;
}

const NStringMap NDatabase::getUserById(int id)
{
    NStringMap user;
    if (id < 0)
        return user;

    QSqlQuery query(m_db);
    QString sql = "SELECT * " \
                  "FROM user " \
                  "WHERE id = :id";

    if (!query.prepare(sql))
    {
        debugLastQuery("getUserById prepare failed", query);
        return user;
    }

    query.bindValue(":id", id);

    if (!query.exec())
    {
        debugLastQuery("getUserById failed", query);
        return user;
    }

    int fieldId = query.record().indexOf("id");
    int fieldEmail = query.record().indexOf("email");
    int fieldPassword = query.record().indexOf("password");
    int fieldName = query.record().indexOf("name");
    int fieldPreferences = query.record().indexOf("preferences");
    int fieldEnabled = query.record().indexOf("enabled");
    int fieldLevel = query.record().indexOf("level");

    if (!query.first())
        return user;

    user["id"] = query.value(fieldId).toString();
    user["email"] = query.value(fieldEmail).toString();
    user["password"] = query.value(fieldPassword).toString();
    user["name"] = query.value(fieldName).toString();
    user["preferences"] = query.value(fieldPreferences).toString();
    user["enabled"] = query.value(fieldEnabled).toString();
    user["level"] =  NTcpServerAuthSession::normalizeLevels(query.value(fieldLevel).toString());

    return user;
}

const NStringMap NDatabase::getUserByEmail(const QString & email)
{
    NStringMap user;
    if (email.isEmpty())
        return user;

    QSqlQuery query(m_db);
    QString sql = "SELECT * " \
                  "FROM user " \
                  "WHERE email = :email";

    if (!query.prepare(sql))
    {
        debugLastQuery("getUserByEmail prepare failed", query);
        return user;
    }

    query.bindValue(":email", email);

    if (!query.exec())
    {
        debugLastQuery("getUserByEmail failed", query);
        return user;
    }

    int fieldId = query.record().indexOf("id");
    int fieldEmail = query.record().indexOf("email");
    int fieldPassword = query.record().indexOf("password");
    int fieldName = query.record().indexOf("name");
    int fieldPreferences = query.record().indexOf("preferences");
    int fieldEnabled = query.record().indexOf("enabled");
    int fieldLevel = query.record().indexOf("level");

    if (!query.first())
        return user;

    user["id"] = query.value(fieldId).toString();
    user["email"] = query.value(fieldEmail).toString();
    user["password"] = query.value(fieldPassword).toString();
    user["name"] = query.value(fieldName).toString();
    user["preferences"] = query.value(fieldPreferences).toString();
    user["enabled"] = query.value(fieldEnabled).toString();
    user["level"] = query.value(fieldLevel).toString();

    return user;
}

bool NDatabase::deleteUser(const QString & id)
{
    if (id.isEmpty())
        return false;

    QSqlQuery query(m_db);
    if (!query.prepare("DELETE FROM user WHERE id=:id"))
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
    return query.numRowsAffected() == 1;
}

void NDatabase::vacuum()
{
    QSqlQuery query(m_db);

    if (!query.prepare("VACUUM"))
    {
        debugLastQuery("NDatabase::vacuum prepare failed", query);
    }

    if (!query.exec())
        debugLastQuery("NMusicDatabase::vacuum exec failed", query);
}
