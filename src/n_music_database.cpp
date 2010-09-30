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
#include <QFileInfo>
#include <QDir>
#include <QSet>

// App
#include "n_path.h"
#include "n_file_category.h"
#include "n_file_suffix.h"
#include "n_config.h"
#include "n_log.h"
#include "n_convert.h"
#include "n_database.h"
#include "n_sqlite_error.h"

#include "n_music_database.h"

// drop table music_album; drop table music_artist; drop table music_album_title; drop table music_genre; drop table music_title;

NMusicDatabase * NMusicDatabase::m_instance = NULL;

NMusicDatabase & NMusicDatabase::instance(QSqlDatabase *db)
{
    if (m_instance == NULL)
    {
        Q_ASSERT(db);
        m_instance = new NMusicDatabase(db);
    }
    return *m_instance;
}

void NMusicDatabase::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NMusicDatabase::NMusicDatabase(QSqlDatabase *db)
    :m_db(db)
{
    Q_ASSERT(m_db);
    createTables();
}

void NMusicDatabase::createTables()
{
    createAlbumTable();
    createArtistTable();
    createGenreTable();
    createTitleTable();
    createAlbumTitleTable();
}

NMusicDatabase::~NMusicDatabase()
{
}

void NMusicDatabase::updateDb(){
    updateAlbumTable();
    updateArtistTable();
    updateGenreTable();
    updateTitleTable();
    updateAlbumTitleTable();

    NDB.beginTransaction();
    updateAlbumCover();
    NDB.commitTransaction();
}

void NMusicDatabase::createAlbumTable()
{
    QSqlQuery query(*m_db);

    // TODO: delete entries when no more used
    // TODO: clear file_hash when file hash are deleted
    // Delete after update
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_album (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "name TEXT UNIQUE,"\
            "main_artist TEXT,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL," \
            "front_cover_picture_file_hash TEXT," \
            "back_cover_picture_file_hash TEXT," \
            "front_cover_id3picture_file_hash TEXT," \
            "back_cover_id3picture_file_hash TEXT" \
            ")"))
        NDatabase::debugLastQuery("music_album table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_album_index "\
                    "ON music_album(name)"))
        NDatabase::debugLastQuery("idx_music_album_index creation failed", query);
}


bool NMusicDatabase::updateAlbumTable()
{
    logDebug("NMusicDatabase", "updateAlbumTable start");
    NDB.beginTransaction();
    if (!setAlbumDeleted())
    {
        NDB.abortTransaction();
        return false;
    }

    if (!populateAlbum())
    {
        NDB.abortTransaction();
        return false;
    }

    if (!removeDeletedAlbum())
    {
        NDB.abortTransaction();
        return false;
    }

    NDB.commitTransaction();
    logDebug("NMusicDatabase", "updateAlbumTable ends");
    return true;
}

bool NMusicDatabase::setAlbumDeleted(const QString albumName, bool deleted)
{
    QSqlQuery query(*m_db);

    QString sql = "UPDATE music_album "\
                  "SET deleted=:deleted";

    if (!albumName.isEmpty())
        sql += " WHERE name=:name";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("setAlbumDeleted prepare failed", query);
        return false;
    }

    query.bindValue(":deleted", deleted ? 1 : 0);
    if (!albumName.isEmpty())
        query.bindValue(":name", albumName);

    if (!query.exec())
    {
        NDatabase::debugLastQuery("setAlbumDeleted failed", query);
        return false;
    }
    return true;
}

bool NMusicDatabase::populateAlbum()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT DISTINCT album, artist, ("\
                  "    SELECT COUNT(DISTINCT artist) "\
                  "    FROM file_metadata AS fm "\
                  "    WHERE fm.album = file_metadata.album"\
                  ") AS artist_number "\
                  "FROM file, file_metadata "\
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND file.hash <> ''";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("populateAlbum prepare failed", query);
        return false;
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("populateAlbum failed", query);
        return false;
    }

    int albumField = query.record().indexOf("album");
    int artistField = query.record().indexOf("artist");
    int artistNumberField = query.record().indexOf("artist_number");
    QSet<QString/*album*/> addedAlbums;
    QString album, mainArtist;
    while (query.next()) {
        mainArtist.clear();
        if (query.value(artistNumberField).toInt() == 1)
            mainArtist = query.value(artistField).toString();
        album = query.value(albumField).toString();
        if (addedAlbums.contains(album))
            continue;
        addedAlbums.insert(album);
        if (!insertAlbum(album, mainArtist))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertAlbum(const QString & albumName, const QString & mainArtistName)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("INSERT INTO music_album (name, main_artist) "\
                       "VALUES(:name, :mainArtist)"))
    {
        NDatabase::debugLastQuery("insertAlbum prepare failed", query);
        return false;
    }

    query.bindValue(":name", albumName);
    query.bindValue(":mainArtist", mainArtistName);

    if (!query.exec())
    {
        if (query.lastError().number() == SQLITE_CONSTRAINT)
            return setAlbumDeleted(albumName, false);
        NDatabase::debugLastQuery("insertAlbum failed", query);
        return false;
    }

    return true;
}


bool NMusicDatabase::removeDeletedAlbum()
{
    QSqlQuery query(*m_db);
    if (!query.exec("DELETE FROM music_album WHERE deleted=1"))
    {
        NDatabase::debugLastQuery("removeDeletedAlbum failed", query);
        return false;
    }
    return true;
}

void NMusicDatabase::createArtistTable()
{
    QSqlQuery query(*m_db);

    // TODO: delete entries when no more used
    // Delete on update
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_artist (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "name TEXT UNIQUE,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL" \
            ")"))
        NDatabase::debugLastQuery("music_artist table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_artist_index "\
                    "ON music_artist(name)"))
        NDatabase::debugLastQuery("idx_music_artist_index creation failed", query);
}


bool NMusicDatabase::updateArtistTable()
{
    logDebug("NMusicDatabase", "updateArtistTable start");
    NDB.beginTransaction();
    if (!setArtistDeleted())
    {
        NDB.abortTransaction();
        return false;
    }

    if (!populateArtist())
    {
        NDB.abortTransaction();
        return false;
    }

    if (!removeDeletedArtist())
    {
        NDB.abortTransaction();
        return false;
    }

    NDB.commitTransaction();
    logDebug("NMusicDatabase", "updateArtistTable ends");
    return true;
}

bool NMusicDatabase::setArtistDeleted(const QString artistName, bool deleted)
{
    QSqlQuery query(*m_db);

    QString sql = "UPDATE music_artist "\
                  "SET deleted=:deleted";

    if (!artistName.isEmpty())
        sql += " WHERE name=:name";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("setArtistDeleted prepare failed", query);
        return false;
    }

    query.bindValue(":deleted", deleted ? 1 : 0);
    if (!artistName.isEmpty())
        query.bindValue(":name", artistName);

    if (!query.exec())
    {
        NDatabase::debugLastQuery("setArtistDeleted failed", query);
        return false;
    }
    return true;
}

bool NMusicDatabase::populateArtist()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT DISTINCT file_metadata.artist "\
                  "FROM file_metadata, file "\
                  "WHERE file_metadata.artist IS NOT NULL "
                  "AND file_metadata.id = file.fk_file_metadata_id "\
                  "AND file.hash <> '' ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("populateArtist prepare failed", query);
        return false;
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("populateArtist failed", query);
        return false;
    }

    int field = query.record().indexOf("artist");

    while (query.next()) {

        if (!insertArtist(query.value(field).toString()))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertArtist(const QString & artistName)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("INSERT INTO music_artist(name) "\
                       "VALUES(:name)"))
    {
        NDatabase::debugLastQuery("insertArtist prepare failed", query);
        return false;
    }

    query.bindValue(":name", artistName);

    if (!query.exec())
    {
        if (query.lastError().number() == SQLITE_CONSTRAINT)
            return setArtistDeleted(artistName, false);
       NDatabase::debugLastQuery("insertArtist failed", query);
        return false;
    }

    return true;
}


bool NMusicDatabase::removeDeletedArtist()
{
    QSqlQuery query(*m_db);
    if (!query.exec("DELETE FROM music_artist WHERE deleted=1"))
    {
        NDatabase::debugLastQuery("removeDeletedArtist failed", query);
        return false;
    }
    return true;
}


void NMusicDatabase::createGenreTable()
{
    QSqlQuery query(*m_db);

    // TODO: delete entries when no more used
    // Delete on update
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_genre (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "name TEXT UNIQUE,"\
            "deleted BOOLEAN DEFAULT 0 NOT NULL" \
            ")"))
        NDatabase::debugLastQuery("music_genre table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_genre_index "\
                    "ON music_genre(name)"))
        NDatabase::debugLastQuery("idx_music_genre_index creation failed", query);
}


bool NMusicDatabase::updateGenreTable()
{
    logDebug("NMusicDatabase", "updateGenreTable start");
    NDB.beginTransaction();
    if (!setGenreDeleted())
    {
        NDB.abortTransaction();
        return false;
    }

    if (!populateGenre())
    {
        NDB.abortTransaction();
        return false;
    }

    if (!removeDeletedGenre())
    {
        NDB.abortTransaction();
        return false;
    }

    NDB.commitTransaction();
    logDebug("NMusicDatabase", "updateGenreTable ends");
    return true;
}

bool NMusicDatabase::setGenreDeleted(const QString genreName, bool deleted)
{
    QSqlQuery query(*m_db);

    QString sql = "UPDATE music_genre "\
                  "SET deleted=:deleted";

    if (!genreName.isEmpty())
        sql += " WHERE name=:name";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("setGenreDeleted prepare failed", query);
        return false;
    }

    query.bindValue(":deleted", deleted ? 1 : 0);
    if (!genreName.isEmpty())
        query.bindValue(":name", genreName);

    if (!query.exec())
    {
        NDatabase::debugLastQuery("setGenreDeleted failed", query);
        return false;
    }
    return true;
}

bool NMusicDatabase::populateGenre()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT DISTINCT file_metadata.genre "\
                  "FROM file_metadata, file  "\
                  "WHERE file_metadata.genre IS NOT NULL "\
                  "AND file_metadata.id = file.fk_file_metadata_id "\
                  "AND file.hash <> '' ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("populateGenre prepare failed", query);
        return false;
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("populateGenre failed", query);
        return false;
    }

    int field = query.record().indexOf("genre");

    while (query.next()) {

        if (!insertGenre(query.value(field).toString()))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertGenre(const QString & genreName)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("INSERT INTO music_genre(name) "\
                       "VALUES(:name)"))
    {
        NDatabase::debugLastQuery("insertGenre prepare failed", query);
        return false;
    }

    query.bindValue(":name", genreName);

    if (!query.exec())
    {
        if (query.lastError().number() == SQLITE_CONSTRAINT)
            return setGenreDeleted(genreName, false);

        NDatabase::debugLastQuery("insertGenre failed", query);
        return false;
    }

    return true;
}


bool NMusicDatabase::removeDeletedGenre()
{
    QSqlQuery query(*m_db);
    if (!query.exec("DELETE FROM music_genre WHERE deleted=1"))
    {
        NDatabase::debugLastQuery("removeDeletedGenre failed", query);
        return false;
    }
    return true;
}

void NMusicDatabase::createTitleTable()
{
    QSqlQuery query(*m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_title (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fk_file_id INTEGER UNIQUE NOT NULL,"\
            "fk_music_artist_id INTEGER NOT NULL," \
            "fk_music_genre_id INTEGER NOT NULL," \
            "title TEXT," \
            "duration INTEGER," \
            "track_number INTEGER," \
            "year INTEGER," \
            "comment TEXT," \
            "has_id3_picture BOOLEAN DEFAULT 0 NOT NULL,"\
            "FOREIGN KEY (fk_file_id) REFERENCES file(id) ON DELETE CASCADE,"\
            "FOREIGN KEY (fk_music_artist_id) REFERENCES music_artist(id) ON DELETE CASCADE,"\
            "FOREIGN KEY (fk_music_genre_id) REFERENCES music_genre(id) ON DELETE CASCADE"
            ")"))
        NDatabase::debugLastQuery("music_title table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_title_index "\
                    "ON music_title(fk_music_artist_id, fk_music_genre_id, fk_file_id)"))
        NDatabase::debugLastQuery("idx_music_title_index creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_title_index_2 "\
                    "ON music_title(fk_music_genre_id, fk_music_artist_id, fk_file_id, id)"))
        NDatabase::debugLastQuery("idx_music_title_index_2 creation failed", query);
}


bool NMusicDatabase::updateTitleTable()
{
    // Music title are deleted from reference to file table.
    // If a file is deleted, title is too.
    logDebug("NMusicDatabase", "updateTitleTable start");
    NDB.beginTransaction();

    if (!populateTitle())
    {
        NDB.abortTransaction();
        return false;
    }

    NDB.commitTransaction();
    logDebug("NMusicDatabase", "updateTitleTable ends");
    return true;
}

bool NMusicDatabase::populateTitle()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT file.id id, file_metadata.genre, file_metadata.artist, "\
                  "       file_metadata.title, file_metadata.duration, file_metadata.track_number, "\
                  "       file_metadata.year, file_metadata.comment, file_metadata.has_id3_picture "\
                  "FROM file, file_metadata " \
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND file.fk_file_category_id = :fk_file_category_id "\
                  "AND file.hash <> '' ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("populateTitle prepare failed", query);
        return false;
    }

    query.bindValue(":fk_file_category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcMusic));

    if (!query.exec())
    {
        NDatabase::debugLastQuery("populateTitle failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
    int fieldGenre = query.record().indexOf("genre");
    int fieldArtist = query.record().indexOf("artist");
    int fieldTitle = query.record().indexOf("title");
    int fieldDuration = query.record().indexOf("duration");
    int fieldTrackNumber = query.record().indexOf("track_number");
    int fieldYear = query.record().indexOf("year");
    int fieldComment = query.record().indexOf("comment");
    int fieldHasId3Picture = query.record().indexOf("has_id3_picture");

    while (query.next()) {
        if (!insertTitle(query.value(fieldId).toInt(),
                         query.value(fieldGenre).toString(),
                         query.value(fieldArtist).toString(),
                         query.value(fieldTitle).toString(),
                         query.value(fieldDuration).toInt(),
                         query.value(fieldTrackNumber).toInt(),
                         query.value(fieldYear).toInt(),
                         query.value(fieldComment).toString(),
                         query.value(fieldHasId3Picture).toBool()))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertTitle(int fileId, const QString & genre,
                                 const QString & artist, const QString & title,
                                 int duration, int trackNumber, int year,
                                 const QString & comment, bool hasId3Picture)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("INSERT INTO music_title(fk_file_id, fk_music_genre_id, "\
                       "                        fk_music_artist_id, title, duration, "\
                       "                        track_number, year, comment, has_id3_picture) "\
                       "VALUES("\
                       "  :fileId, "\
                       "  (SELECT id FROM music_genre where name=:genre),"\
                       "  (SELECT id FROM music_artist where name=:artist),"\
                       "  :title, :duration, :track_number, :year, :comment,"\
                       "  :has_id3_picture"
                       ")"))
    {
        NDatabase::debugLastQuery("insertTitle prepare failed", query);
        return false;
    }

    query.bindValue(":fileId", fileId);
    query.bindValue(":genre", genre);
    query.bindValue(":artist", artist);

    query.bindValue(":title", title);
    query.bindValue(":duration", duration);
    query.bindValue(":track_number", trackNumber);
    query.bindValue(":year", year);
    query.bindValue(":comment", comment);
    query.bindValue(":has_id3_picture", hasId3Picture);
    if (!query.exec())
    {
        if (query.lastError().number() == SQLITE_CONSTRAINT)
            return true;
        NDatabase::debugLastQuery("insertTitle failed", query);
        return false;
    }

    return true;
}

void NMusicDatabase::createAlbumTitleTable()
{
    QSqlQuery query(*m_db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_album_title (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fk_music_title_id INTEGER UNIQUE NOT NULL," \
            "fk_music_album_id INTEGER NOT NULL," \
            "FOREIGN KEY (fk_music_title_id) REFERENCES music_title(id) ON DELETE CASCADE," \
            "FOREIGN KEY (fk_music_album_id) REFERENCES music_album(id) ON DELETE CASCADE"
            ")"))
        NDatabase::debugLastQuery("music_album_title table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_album_title_index "\
                    "ON music_album_title(fk_music_album_id, fk_music_title_id)"))
        NDatabase::debugLastQuery("idx_music_album_title_index creation failed", query);
}

bool NMusicDatabase::updateAlbumTitleTable()
{
    logDebug("NMusicDatabase", "updateAlbumTitleTable start");
    NDB.beginTransaction();

    if (!populateAlbumTitle())
    {
        NDB.abortTransaction();
        return false;
    }

    NDB.commitTransaction();
    logDebug("NMusicDatabase", "updateAlbumTitleTable ends");
    return true;
}

bool NMusicDatabase::populateAlbumTitle()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT music_title.id, file_metadata.album "\
                  "FROM music_title, file, file_metadata "\
                  "WHERE music_title.fk_file_id = file.id "\
                  "AND file.fk_file_metadata_id = file_metadata.id ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("populateAlbumTitle prepare failed", query);
        return false;
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("populateAlbumTitle failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
    int fieldAlbum = query.record().indexOf("album");

    while (query.next()) {
        if (!insertAlbumTitle(query.value(fieldId).toInt(), query.value(fieldAlbum).toString()))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertAlbumTitle(int titleId, const QString & albumName)
{
    QSqlQuery query(*m_db);
    QString sql = "INSERT INTO music_album_title(fk_music_title_id, fk_music_album_id) "\
                  "VALUES(:titleId, (%1))";

    if (!albumName.isEmpty())
        sql = QString(sql).arg("SELECT id FROM music_album WHERE name = :album");
    else
        sql = QString(sql).arg("SELECT id FROM music_album WHERE (name IS NULL OR name = '')");

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("insertAlbumTitle prepare failed", query);
        return false;
    }

    query.bindValue(":titleId", titleId);
    if (!albumName.isEmpty())
        query.bindValue(":album", albumName);
    if (!query.exec())
    {
        if (query.lastError().number() == SQLITE_CONSTRAINT)
            return true;
        NDatabase::debugLastQuery("insertAlbumTitle failed", query);
        return true;
    }

    return true;
}


bool NMusicDatabase::updateAlbumCover()
{
    logDebug("NMusicDatabase", "updateAlbumCover start");
    // TODO: delete file hash reference in music_album table when removing a music
    QSqlQuery query(*m_db);
    QString sql;

    sql = "SELECT music_album.id, file.hash, file.absolute_file_path, file_metadata.has_id3_picture "\
          "FROM music_album, music_album_title, music_title, file, file_metadata "\
          "WHERE music_album.name <> \"\" "
          "AND music_album.id = music_album_title.fk_music_album_id "\
          "AND music_album_title.fk_music_title_id = music_title.id "\
          "AND music_title.fk_file_id = file.id "\
          "AND file.fk_file_metadata_id = file_metadata.id "\
          "GROUP BY music_album.name, file.relative_path";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("updateAlbumCover prepare failed", query);
        return false;
    }

    if (!query.exec())
    {
        // uncomment for DEBUG if needed
        NDatabase::debugLastQuery("updateAlbumCover failed", query);
        return true;
    }

    int albumIdFieldIdx = query.record().indexOf("id");
    int absoluteFilePathFieldIdx = query.record().indexOf("absolute_file_path");
    int hasId3PictureFieldIdx = query.record().indexOf("has_id3_picture");
    int hashFieldIdx = query.record().indexOf("hash");

    QSqlQuery q(*m_db);
    QString sql2;
    sql2 = "SELECT hash, absolute_file_path "\
           "FROM file "\
           "WHERE fk_file_category_id=:fk_file_category_id "\
           "AND absolute_file_path LIKE :absoluteFilePath "\
           "AND (file_name LIKE \"%front%\" "\
           "OR file_name LIKE \"%cover%\" "\
           "OR file_name LIKE \"%albumart%\" "\
           "OR file_name LIKE \"%large%\") "\
           "ORDER BY size DESC";

    if (!q.prepare(sql2))
    {
        NDatabase::debugLastQuery("updateAlbumCover (2) prepare failed", q);
        return false;
    }

    // Files to add
    QHash<int/*album id*/, QString/*hash*/> fcifhHash; // File cover id3 file hash
    QHash<int/*album id*/, QString/*hash*/> fcpfhHash; // File cover picture file hash

    while (query.next()) {
        int albumId = query.value(albumIdFieldIdx).toInt();

        // front cover id3picture file hash
        if (!fcifhHash.contains(albumId)){
            bool hasId3Picture = query.value(hasId3PictureFieldIdx).toBool();
            if (hasId3Picture)
                fcifhHash[albumId] = query.value(hashFieldIdx).toString();
        }


        // Front cover picture file hash
        if (!fcpfhHash.contains(albumId))
        {
            QString absoluteFilePath = query.value(absoluteFilePathFieldIdx).toString();
            QFileInfo fi(absoluteFilePath);
            QDir dir = fi.absoluteDir();

            q.bindValue(":fk_file_category_id", NFileCategory_n::fileCategoryId(NFileCategory_n::fcPicture));
            q.bindValue(":absoluteFilePath", QString("%1%").arg(dir.absolutePath()));
            //logDebug("dir.absolutePath()", dir.absolutePath());

            if (!q.exec())
            {
                NDatabase::debugLastQuery("updateAlbumCover (2) failed", q);
                return true;
            }
            int hashIdx = q.record().indexOf("hash");
            int absoluteFilePathIdx = q.record().indexOf("absolute_file_path");
            while (q.next()) {
                QString picAbsoluteFilePath = q.value(absoluteFilePathIdx).toString();
                QFileInfo fiAbsoluteFilePath(picAbsoluteFilePath);
                if (dir == fiAbsoluteFilePath.dir()){
                    QString picHash = q.value(hashIdx).toString();
                    // TODO: use a regex
                    // TODO: do a real selection between pics in dir
                    // TODO: back cover, cd cover...
                    fcpfhHash[albumId] = picHash;
                    break;
                }
            }
        }
    }

    insertAlbumFrontCoverPictureFileHash(fcpfhHash);
    insertAlbumFrontCoverId3Picture(fcifhHash);

    logDebug("NMusicDatabase", "updateAlbumCover ends");
    return true;
}


bool NMusicDatabase::getAlbumList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                                  const QStringList & searches, int start, int limit,
                                  const QString & dir, int year, const QString & genre,
                                  const QString & artist)
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT music_album.id id, music_album.name album, music_album.main_artist artist, "\
                  "       music_album.front_cover_picture_file_hash fcpfh, "\
                  "       music_album.back_cover_picture_file_hash bcpfh, "\
                  "       music_album.front_cover_id3picture_file_hash fcipfh, "\
                  "       music_album.back_cover_id3picture_file_hash bcipfh "\
                  "FROM music_album ";

    if (year >= 0 || !genre.isNull() || !artist.isNull() || searches.count()) {
        sql += "  INNER JOIN music_album_title "\
               "    ON music_album.id = music_album_title.fk_music_album_id "\
               "  INNER JOIN music_title "\
               "    ON music_title.id = music_album_title.fk_music_title_id ";
    }

    if (!genre.isNull() || searches.count()){
        sql += "  INNER JOIN music_genre "\
               "    ON music_title.fk_music_genre_id = music_genre.id ";
    }

    if (!artist.isNull() || searches.count()){
        sql += "  INNER JOIN music_artist "\
               "    ON music_title.fk_music_artist_id = music_artist.id ";
    }

    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "music_title.year = :year ";
    }

    if (!genre.isNull()){
        NDatabase::addAND(sql, &AND);
        if (genre.isEmpty())
            sql += "(music_genre.name = '' OR  music_genre.name is NULL) ";
        else
            sql += "music_genre.name = :genre ";
    }
    if (!artist.isNull()){
        NDatabase::addAND(sql, &AND);
        if (artist.isEmpty())
            sql += "(music_artist.name = '' OR  music_artist.name is NULL) ";
        else
        sql += "music_artist.name = :artist ";
    }
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort
    sql += QString("GROUP BY music_album.id ORDER BY music_album.name %1 ").
           arg(NDatabase::stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getAlbumList prepare failed", query);
        return false;
    }

    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isEmpty())
        query.bindValue(":genre", genre);
    if (!artist.isEmpty())
        query.bindValue(":artist", artist);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getAlbumList failed", query);
        return false;
    }/* else {
        logDebug("NMusicDatabase", query.lastQuery());
    }*/
    logDebug("NMusicDatabase", query.lastQuery());

    int fieldId = query.record().indexOf("id");
    int fieldAlbum = query.record().indexOf("album");
    int fieldMainArtist = query.record().indexOf("artist");
    int fieldFrontCoverPicture = query.record().indexOf("fcpfh");
    int fieldBackCoverPicture = query.record().indexOf("bcpfh");
    int fieldFrontCoverID3Picture = query.record().indexOf("fcipfh");
    int fieldBackCoverID3Picture = query.record().indexOf("bcipfh");

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
        // TODO: return id: cos album can be empty
        svAlbum.setProperty("id", -1);
        svAlbum.setProperty("album", QScriptValue("album-all"));
        svAlbum.setProperty("mainArtist", "");
        svAlbum.setProperty("frontCoverPictureFileHash", "");
        svAlbum.setProperty("backCoverPictureFileHash", "");
        svAlbum.setProperty("frontCoverID3PictureFileHash", "");
        svAlbum.setProperty("backCoverID3PictureFileHash", "");
    }

    while (query.next()) {
        QScriptValue svAlbum = se.newObject();
        dataArray.setProperty(i, svAlbum);
        svAlbum.setProperty("id", query.value(fieldId).toInt());
        svAlbum.setProperty("album", query.value(fieldAlbum).toString());
        svAlbum.setProperty("mainArtist", query.value(fieldMainArtist).toString());
        svAlbum.setProperty("frontCoverPictureFileHash", query.value(fieldFrontCoverPicture).toString());
        svAlbum.setProperty("backCoverPictureFileHash", query.value(fieldBackCoverPicture).toString());
        svAlbum.setProperty("frontCoverID3PictureFileHash", query.value(fieldFrontCoverID3Picture).toString());
        svAlbum.setProperty("backCoverID3PictureFileHash", query.value(fieldBackCoverID3Picture).toString());
        i++;
    }

    if (start + limit >= totalCount &&
        totalCount >= 2 &&
        dir.compare("ASC", Qt::CaseInsensitive))
    {
        QScriptValue svAlbum = se.newObject();
        dataArray.setProperty(i, svAlbum);
        i++;
        svAlbum.setProperty("id", -1);
        svAlbum.setProperty("album", QScriptValue("album-all"));
        svAlbum.setProperty("mainArtist", "");
        svAlbum.setProperty("frontCoverPictureFileHash", "");
        svAlbum.setProperty("backCoverPictureFileHash", "");
        svAlbum.setProperty("frontCoverID3PictureFileHash", "");
        svAlbum.setProperty("backCoverID3PictureFileHash", "");
    }

    return true;
}

int NMusicDatabase::getAlbumListCount(const QStringList & searches, int year, const QString & genre,
                                      const QString & artist)
{
    NStringMapList list;
    QSqlQuery query(*m_db);

    QString sql = "SELECT count(DISTINCT music_album.id) "\
                  "FROM music_album ";

    if (year >= 0 || !genre.isNull() || !artist.isNull() || searches.count()) {
        sql += "  INNER JOIN music_album_title "\
               "    ON music_album.id = music_album_title.fk_music_album_id "\
               "  INNER JOIN music_title "\
               "    ON music_title.id = music_album_title.fk_music_title_id ";
    }

    if (!genre.isNull() || searches.count()){
        sql += "  INNER JOIN music_genre "\
               "    ON music_title.fk_music_genre_id = music_genre.id ";
    }

    if (!artist.isNull() || searches.count()){
        sql += "  INNER JOIN music_artist "\
               "    ON music_title.fk_music_artist_id = music_artist.id ";
    }

    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "music_title.year = :year ";
    }

    if (!genre.isNull()){
        NDatabase::addAND(sql, &AND);
        if (genre.isEmpty())
            sql += "(music_genre.name = '' OR  music_genre.name is NULL) ";
        else
            sql += "music_genre.name = :genre ";
    }
    if (!artist.isNull()){
        NDatabase::addAND(sql, &AND);
        if (artist.isEmpty())
            sql += "(music_artist.name = '' OR  music_artist.name is NULL) ";
        else
        sql += "music_artist.name = :artist ";
    }
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getAlbumListCount prepare failed", query);
        return 0;
    }

    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isEmpty())
        query.bindValue(":genre", genre);
    if (!artist.isEmpty())
        query.bindValue(":artist", artist);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getAlbumListCount failed", query);
        return 0;
    }/* else {
        logDebug("NMusicDatabase", query.lastQuery());
    }*/
    logDebug("NMusicDatabase", query.lastQuery());

    if (!query.first())
        return 0;

    // We add 1 to result to manage "album-all"
    // "artist-all" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NMusicDatabase::getArtistList(QScriptEngine & se, QScriptValue & dataArray,
                                   int totalCount, const QStringList & searches, int start,
                                   int limit, const QString & dir, int year,
                                   const QString & genre)
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT music_artist.id id, music_artist.name artist "\
                  "FROM music_artist ";

    if (year >= 0 || !genre.isNull() || searches.count()) {
        sql += "  INNER JOIN music_title "\
               "    ON music_title.fk_music_artist_id = music_artist.id ";
    }

    if (!genre.isNull() || searches.count()) {
        sql += "  INNER JOIN music_genre "\
               "    ON music_title.fk_music_genre_id = music_genre.id ";
    }

    if (searches.count()) {
        sql += "  INNER JOIN music_album_title "\
               "    ON music_album_title.fk_music_title_id = music_title.id "\
               "  INNER JOIN music_album "\
               "    ON music_album.id = music_album_title.fk_music_album_id ";
    }

    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "music_title.year = :year ";
    }

    if (!genre.isNull()){
        NDatabase::addAND(sql, &AND);
        if (genre.isEmpty())
            sql += "(music_genre.name = '' OR  music_genre.name is NULL) ";
        else
            sql += "music_genre.name = :genre ";
    }


    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("GROUP BY music_artist.id ORDER BY music_artist.name %1 ").
           arg(NDatabase::stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getArtistList prepare failed", query);
        return false;
    }

    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isEmpty())
        query.bindValue(":genre", genre);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getArtistList failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
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
        svArtist.setProperty("id", -1);
        svArtist.setProperty("artist", QScriptValue("artist-all"));
    }

    while (query.next()) {
        QScriptValue svArtist = se.newObject();
        dataArray.setProperty(i, svArtist);
        svArtist.setProperty("id", query.value(fieldId).toInt());
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
        svArtist.setProperty("id", -1);
        svArtist.setProperty("artist", QScriptValue("artist-all"));
    }

    return true;
}

int NMusicDatabase::getArtistListCount(const QStringList & searches, int year, const QString & genre)
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT count(DISTINCT music_artist.id) "\
                  "FROM music_artist ";

    if (year >= 0 || !genre.isNull() || searches.count()) {
        sql += "  INNER JOIN music_title "\
               "    ON music_title.fk_music_artist_id = music_artist.id ";
    }

    if (!genre.isNull() || searches.count()) {
        sql += "  INNER JOIN music_genre "\
               "    ON music_title.fk_music_genre_id = music_genre.id ";
    }

    if (searches.count()) {
        sql += "  INNER JOIN music_album_title "\
               "    ON music_album_title.fk_music_title_id = music_title.id "\
               "  INNER JOIN music_album "\
               "    ON music_album.id = music_album_title.fk_music_album_id ";
    }

    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "music_title.year = :year ";
    }

    if (!genre.isNull()){
        NDatabase::addAND(sql, &AND);
        if (genre.isEmpty())
            sql += "(music_genre.name = '' OR  music_genre.name is NULL) ";
        else
            sql += "music_genre.name = :genre ";
    }
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getArtistListCount prepare failed", query);
        return 0;
    }

    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isEmpty())
        query.bindValue(":genre", genre);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getArtistListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;

    // We add 1 to result to manage "artist-all"
    // "artist-all" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NMusicDatabase::getGenreList(QScriptEngine & se, QScriptValue & dataArray,
                                  int totalCount, const QStringList & searches, int start,
                                  int limit, const QString & dir, int year)
{
    QSqlQuery query(*m_db);
    QString sql = "SELECT music_genre.id, music_genre.name genre "\
                  "FROM music_genre ";

    if (year >= 0 || searches.count()) {
        sql += "  INNER JOIN music_title "\
               "    ON music_title.fk_music_genre_id = music_genre.id ";
    }

    if (searches.count()) {
        sql += "  INNER JOIN music_artist "\
               "    ON music_title.fk_music_artist_id = music_artist.id "\
               "  INNER JOIN music_album_title "\
               "    ON music_album_title.fk_music_title_id = music_title.id "\
               "  INNER JOIN music_album "\
               "    ON music_album.id = music_album_title.fk_music_album_id ";
    }

    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "music_title.year = :year ";
    }
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("GROUP BY music_genre.id ORDER BY music_genre.name %1 ").
           arg(NDatabase::stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";


    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getGenreList prepare failed", query);
        return false;
    }

    if (year >= 0)
        query.bindValue(":year", year);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getGenreList failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");
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
        svGenre.setProperty("id", -1);
        svGenre.setProperty("genre", QScriptValue("genre-all"));
        i++;
    }

    while (query.next()) {
        QScriptValue svGenre = se.newObject();
        dataArray.setProperty(i, svGenre);
        svGenre.setProperty("id", query.value(fieldId).toInt());
        svGenre.setProperty("genre", query.value(fieldGenre).toString());
        i++;
    }

    if (start + limit >= totalCount &&
        totalCount >= 2 &&
        dir.compare("ASC", Qt::CaseInsensitive))
    {
        QScriptValue svGenre = se.newObject();
        dataArray.setProperty(i, svGenre);
        svGenre.setProperty("id", -1);
        svGenre.setProperty("genre", QScriptValue("genre-all"));
        i++;
    }

    return true;
}

int NMusicDatabase::getGenreListCount(const QStringList & searches, int year)
{
    QSqlQuery query(*m_db);
    QString sql = "SELECT count(DISTINCT music_genre.id) "\
                  "FROM music_genre ";

    if (year >= 0 || searches.count()) {
        sql += "  INNER JOIN music_title "\
               "    ON music_title.fk_music_genre_id = music_genre.id ";
    }

    if (searches.count()) {
        sql += "  INNER JOIN music_artist "\
               "    ON music_title.fk_music_artist_id = music_artist.id "\
               "  INNER JOIN music_album_title "\
               "    ON music_album_title.fk_music_title_id = music_title.id "\
               "  INNER JOIN music_album "\
               "    ON music_album.id = music_album_title.fk_music_album_id ";
    }

    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "music_title.year = :year ";
    }
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getGenreListCount prepare failed", query);
        return 0;
    }

    if (year >= 0)
        query.bindValue(":year", year);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getGenreListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;

    // We add 1 to result to manage "genre-all"
    // "genre-all" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NMusicDatabase::getYearList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                                 const QStringList & searches, int start,
                                 int limit, const QString & dir)
{
    QSqlQuery query(*m_db);
    QString sql = "SELECT music_title.year "\
                  "FROM music_title ";

    if (searches.count()) {
        sql +=  "  INNER JOIN music_genre "\
                "    ON music_title.fk_music_genre_id = music_genre.id "\
                "  INNER JOIN music_artist "\
                "    ON music_title.fk_music_artist_id = music_artist.id "\
                "  INNER JOIN music_album_title "\
                "    ON music_album_title.fk_music_title_id = music_title.id "\
                "  INNER JOIN music_album "\
                "    ON music_album.id = music_album_title.fk_music_album_id ";
    }

    bool AND = false;
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }

        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("GROUP BY music_title.year ORDER BY music_title.year %1 ").
           arg(NDatabase::stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getYearList prepare failed", query);
        return false;
    }

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getYearList failed", query);
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

int NMusicDatabase::getYearListCount(const QStringList & searches)
{
    QSqlQuery query(*m_db);    
    QString sql = "SELECT count(DISTINCT music_title.year) "\
                  "FROM music_title ";

    if (searches.count()) {
        sql +=  "  INNER JOIN music_genre "\
                "    ON music_title.fk_music_genre_id = music_genre.id "\
                "  INNER JOIN music_artist "\
                "    ON music_title.fk_music_artist_id = music_artist.id "\
                "  INNER JOIN music_album_title "\
                "    ON music_album_title.fk_music_title_id = music_title.id "\
                "  INNER JOIN music_album "\
                "    ON music_album.id = music_album_title.fk_music_album_id ";
    }

    bool AND = false;
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getYearListCount prepare failed", query);
        return 0;
    }

    for (int i = 0; i < searches.count(); ++i){
         searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getYearListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;

    // We add 1 to result to manage "-1"
    // "-1" value is not returned if number of value <= 1
    int total = query.value(0).toInt();
    return total <= 1 ? total : total + 1;
}

bool NMusicDatabase::getTitleList(QScriptEngine & se, QScriptValue & dataArray,
                                  const QStringList & searches, const QString & album,
                                  const QString & artist, const QString & genre,
                                  int year, int start, int limit, const QString & sort,
                                  const QString & dir)
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT music_title.id id, music_title.title, music_title.comment, music_title.year, "\
                  "       music_title.track_number, music_title.duration, music_title.has_id3_picture, "\
                  "       music_artist.name artist, music_genre.name genre, "\
                  "       file.file_name, file.hash, file.size, "\
                  "       music_album.name album, music_album.front_cover_picture_file_hash fcpfh, "\
                  "       music_album.front_cover_id3picture_file_hash fcifh "\
                  "FROM music_title "\
                  "  INNER JOIN music_genre "\
                  "    ON music_title.fk_music_genre_id = music_genre.id "\
                  "  INNER JOIN music_artist "\
                  "    ON music_title.fk_music_artist_id = music_artist.id "\
                  "  INNER JOIN file "\
                  "    ON music_title.fk_file_id = file.id "\
                  "  INNER JOIN music_album_title "\
                  "    ON music_album_title.fk_music_title_id = music_title.id "\
                  "  INNER JOIN music_album "\
                  "    ON music_album.id = music_album_title.fk_music_album_id ";
    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "year = :year ";
    }
    if (!album.isNull()){
        NDatabase::addAND(sql, &AND);
        if (album.isEmpty())
            sql += "(music_album.name = '' OR  music_album.name is NULL) ";
        else
            sql += "music_album.name = :album ";
    }
    if (!genre.isNull()){
        NDatabase::addAND(sql, &AND);
        if (genre.isEmpty())
            sql += "(music_genre.name = '' OR  music_genre.name is NULL) ";
        else
            sql += "music_genre.name = :genre ";
    }
    if (!artist.isNull()){
        NDatabase::addAND(sql, &AND);
        if (artist.isEmpty())
            sql += "(music_artist.name = '' OR  music_artist.name is NULL) ";
        else
        sql += "music_artist.name = :artist ";
    }
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    QString sortField = jsFileStringToDBFileField(sort);
    if (sortField =="music_album.name")
        sortField += QString(" %1,  music_title.track_number").arg(NDatabase::stringToSortDirection(dir));
    sql += QString(" ORDER BY %1 %2 ").
           arg(sortField).
           arg(NDatabase::stringToSortDirection(dir));

    // limit
    if (limit != -1)
        sql += "LIMIT :limit ";

    // offset
    if (start != 0)
        sql += "OFFSET :offset ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getTitleList prepare failed", query);
        return false;
    }

    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isEmpty())
        query.bindValue(":genre", genre);
    if (!artist.isEmpty())
        query.bindValue(":artist", artist);
    if (!album.isEmpty())
        query.bindValue(":album", album);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    // Limit
    if (limit != -1)
        query.bindValue(":limit", limit);

    // start
    if (start != 0)
        query.bindValue(":offset", start);

    //TMP  if (!query.exec())
    if (!query.exec())
    {
        NDatabase::debugLastQuery("getTitleList failed", query);
        return false;
    }/* else {
        //logDebug("NMusicDatabase", query.lastQuery());
    }*/
    logDebug("NMusicDatabase", query.lastQuery());

    // Files fields
    int fieldId = query.record().indexOf("id");
    int fieldFileName = query.record().indexOf("file_name");
    int fieldHash = query.record().indexOf("hash");
    int fieldSize = query.record().indexOf("size");

    int fieldArtist = query.record().indexOf("artist");
    int fieldComment = query.record().indexOf("comment");
    int fieldYear = query.record().indexOf("year");
    int fieldAlbum = query.record().indexOf("album");
    int fieldTitle = query.record().indexOf("title");
    int fieldGenre = query.record().indexOf("genre");
    int fieldTrackNumber = query.record().indexOf("track_number");
    int fieldDuration = query.record().indexOf("duration");
    int fieldHasID3Picture = query.record().indexOf("has_id3_picture");
    int fieldFcpfh = query.record().indexOf("fcpfh");
    int fieldFcifh = query.record().indexOf("fcifh");

    int i = 0;
    while (query.next()) {
        QScriptValue svTitle = se.newObject();
        dataArray.setProperty(i, svTitle);
        i++;

        svTitle.setProperty("id", query.value(fieldId).toInt());
        svTitle.setProperty("fileName", query.value(fieldFileName).toString());
        svTitle.setProperty("hash", query.value(fieldHash).toString());
        svTitle.setProperty("size", query.value(fieldSize).toString()); //TODO: Should be toULongLong

        // Metadata field
        svTitle.setProperty("artist", query.value(fieldArtist).toString());
        svTitle.setProperty("comment", query.value(fieldComment).toString());
        svTitle.setProperty("year", query.value(fieldYear).toInt());
        svTitle.setProperty("album", query.value(fieldAlbum).toString());
        svTitle.setProperty("title", query.value(fieldTitle).toString());
        svTitle.setProperty("genre", query.value(fieldGenre).toString());
        svTitle.setProperty("trackNumber", query.value(fieldTrackNumber).toInt());
        svTitle.setProperty("duration", query.value(fieldDuration).toInt());
        svTitle.setProperty("hasID3Picture", query.value(fieldHasID3Picture).toString());
        svTitle.setProperty("frontCoverID3PictureFileHash", query.value(fieldFcifh).toString());
        svTitle.setProperty("frontCoverPictureFileHash", query.value(fieldFcpfh).toString());
    }

    return true;
}

int NMusicDatabase::getTitleListCount(const QStringList & searches, const QString & album,
                                      const QString & artist, const QString & genre,
                                      int year)
{
    QSqlQuery query(*m_db);
    QString sql = "SELECT count(music_title.id) "\
                  "FROM music_title "\
                  "  INNER JOIN music_genre "\
                  "    ON music_title.fk_music_genre_id = music_genre.id "\
                  "  INNER JOIN music_artist "\
                  "    ON music_title.fk_music_artist_id = music_artist.id "\
                  "  INNER JOIN file "\
                  "    ON music_title.fk_file_id = file.id "\
                  "  INNER JOIN music_album_title "\
                  "    ON music_album_title.fk_music_title_id = music_title.id "\
                  "  INNER JOIN music_album "\
                  "    ON music_album.id = music_album_title.fk_music_album_id ";
    bool AND = false;

    if (year >= 0) {
        NDatabase::addAND(sql, &AND);
        sql += "year = :year ";
    }
    if (!album.isNull()){
        NDatabase::addAND(sql, &AND);
        if (album.isEmpty())
            sql += "(music_album.name = '' OR  music_album.name is NULL) ";
        else
            sql += "music_album.name = :album ";
    }
    if (!genre.isNull()){
        NDatabase::addAND(sql, &AND);
        if (genre.isEmpty())
            sql += "(music_genre.name = '' OR  music_genre.name is NULL) ";
        else
            sql += "music_genre.name = :genre ";
    }
    if (!artist.isNull()){
        NDatabase::addAND(sql, &AND);
        if (artist.isEmpty())
            sql += "(music_artist.name = '' OR  music_artist.name is NULL) ";
        else
        sql += "music_artist.name = :artist ";
    }
    bool intConv;
    for (int i = 0; i < searches.count(); ++i){
        NDatabase::addAND(sql, &AND);
        searches.at(i).toInt(&intConv);
        if (intConv) {
            sql += QString("(music_title.year = :year%1 ").arg(i);
            sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        } else {
            sql += QString("(music_genre.name LIKE :genre%1 ").arg(i);
        }
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getTitleListCount prepare failed", query);
        return 0;
    }

    if (year >= 0)
        query.bindValue(":year", year);
    if (!genre.isEmpty())
        query.bindValue(":genre", genre);
    if (!artist.isEmpty())
        query.bindValue(":artist", artist);
    if (!album.isEmpty())
        query.bindValue(":album", album);

    for (int i = 0; i < searches.count(); ++i){
        searches.at(i).toInt(&intConv);
        if (intConv) {
             query.bindValue(QString(":year%1").arg(i), QString("%1").arg(searches.at(i)));
        }
        query.bindValue(QString(":genre%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":artist%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":album%1").arg(i), QString("%%1%").arg(searches.at(i)));
        query.bindValue(QString(":title%1").arg(i), QString("%%1%").arg(searches.at(i)));
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("getTitleListCount failed", query);
        return 0;
    }/*else {
        //logDebug("NMusicDatabase", query.lastQuery());
    }*/
    logDebug("NMusicDatabase", query.lastQuery());

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}

QString NMusicDatabase::jsFileStringToDBFileField(const QString & jsString)
{                      
    if (jsString.isEmpty())
        return "";                  

    if (jsString == "size")
        return "music_title.size";

    if (jsString == "title")
        return "music_title.title";

    if (jsString == "comment")
        return "music_title.comment";

    if (jsString == "hasId3Picture")
        return "music_title.has_id3_picture";

    if (jsString == "year")
        return "music_title.year";

    if(jsString == "trackNumber")
        return "music_title.track_number";

    if(jsString == "hasID3Picture")
        return "music_title.has_id3_picture";

    if (jsString == "duration")
        return "music_title.duration";

    // Album
    if (jsString == "album")
        return "music_album.name";

    // Genre
    if (jsString == "genre")
        return "music_genre.name";

    // Artist
    if (jsString == "artist")
        return "music_artist.name";

    // File
    if(jsString == "fileName")
        return "file.file_name";

    if (jsString == "hash")
        return "file.hash";

    if (jsString == "size")
        return "file.size";


    Q_ASSERT_X(false, "NMusicDatabase::jsFileStringToDBFileField", qPrintable(QString("%1 is not mapped" ).arg(jsString)));
    return "added";
}

bool NMusicDatabase::insertAlbumFrontCoverPictureFileHash(const QHash<int/*album id*/, QString/*hash*/> & hash)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("UPDATE music_album "\
                       "set front_cover_picture_file_hash=:fcpfh "\
                       "WHERE id=:id"))
    {
        NDatabase::debugLastQuery("insertAlbumFrontCoverPictureFileHash prepare failed", query);
        return false;
    }

    bool success = true;
    QHashIterator<int, QString> i(hash);
    while (i.hasNext()) {
        i.next();
        query.bindValue(":id", i.key());
        query.bindValue(":fcpfh", i.value());
        if (!query.exec())
        {
            success = false;
            // uncomment for DEBUG if needed
            NDatabase::debugLastQuery("insertAlbumFrontCoverPictureFileHash failed", query);
        }
    }
    return success;
}

bool NMusicDatabase::insertAlbumFrontCoverId3Picture(const QHash<int/*album id*/, QString/*hash*/> & hash)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("UPDATE music_album "\
                       "set front_cover_id3picture_file_hash=:fcifh "\
                       "WHERE id=:id"))
    {
        NDatabase::debugLastQuery("insertAlbumFrontCoverId3Picture prepare failed", query);
        return false;
    }

    bool success = true;
    QHashIterator<int, QString> i(hash);
    while (i.hasNext()) {
        i.next();
        query.bindValue(":id", i.key());
        query.bindValue(":fcifh", i.value());
        if (!query.exec())
        {
            success = false;
            // uncomment for DEBUG if needed
            NDatabase::debugLastQuery("insertAlbumFrontCoverId3Picture failed", query);
        }
    }
    return success;
}
