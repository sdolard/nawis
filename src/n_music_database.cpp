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

#include "n_music_database.h"

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
    //createAlbumCoverTable();
}

NMusicDatabase::~NMusicDatabase()
{
}

void NMusicDatabase::updateDb(){
    // TODO: manage transaction here.

    NMDB.updateAlbumTable();
    NMDB.updateArtistTable();
    NMDB.updateGenreTable();
    NMDB.updateTitleTable();
    NMDB.updateAlbumTitleTable();
    NMDB.updateAlbumCover();
}

void NMusicDatabase::createAlbumTable()
{
    QSqlQuery query(*m_db);

    // TODO: delete entries when no more used
    // Delete after update
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_album (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "name TEXT UNIQUE,"\
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

    QString sql = "SELECT DISTINCT album "\
                  "FROM file, file_metadata "\
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND album IS NOT NULL "\
                  "AND file.hash <> '' ";

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

    int field = query.record().indexOf("album");

    while (query.next()) {

        if (!insertAlbum(query.value(field).toString()))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertAlbum(const QString & albumName)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("INSERT INTO music_album (name) "\
                       "VALUES(:name)"))
    {
        NDatabase::debugLastQuery("insertAlbum prepare failed", query);
        return false;
    }

    query.bindValue(":name", albumName);

    if (!query.exec())
    {
        // uncomment for DEBUG if needed
        //NDatabase::debugLastQuery("insertAlbum failed", query);
        return setAlbumDeleted(albumName, false);
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
                  "FROM file, file_metadata "\
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND file_metadata.artist IS NOT NULL "\
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
        // uncomment for DEBUG if needed
        //NDatabase::debugLastQuery("insertArtist failed", query);
        return setArtistDeleted(artistName, false);
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

    QString sql = "SELECT DISTINCT genre "\
                  "FROM file, file_metadata "\
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND genre IS NOT NULL "\
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
        // uncomment for DEBUG if needed
        //NDatabase::debugLastQuery("insertGenre failed", query);
        return setGenreDeleted(genreName, false);
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

    // TODO: delete entries when no more used
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
             "copyright TEXT" \
            ")"))
        NDatabase::debugLastQuery("music_title table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_title_index "\
                    "ON music_title(fk_music_artist_id, fk_music_genre_id, fk_file_id)"))
        NDatabase::debugLastQuery("idx_music_title_index creation failed", query);

    if (!query.exec(
            "CREATE TRIGGER IF NOT EXISTS delete_music_title " \
            "BEFORE DELETE ON file "\
            "FOR EACH ROW BEGIN "\
            "  DELETE from music_title WHERE fk_file_id = OLD.id; "\
            "END; "))
        NDatabase::debugLastQuery("file CREATE TRIGGER delete_music_title failed", query);
}


bool NMusicDatabase::updateTitleTable()
{
    NDB.beginTransaction();

    if (!populateTitle())
    {
        NDB.abortTransaction();
        return false;
    }

    NDB.commitTransaction();
    return true;
}

bool NMusicDatabase::populateTitle()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT file.id id, file_metadata.genre, file_metadata.artist, "\
                  "       file_metadata.title, file_metadata.duration, file_metadata.track_number, "\
                  "       file_metadata.year, file_metadata.comment, file_metadata.has_id3_picture, "\
                  "       file_metadata.copyright "\
                  "FROM file, file_metadata " \
                  "WHERE file.fk_file_metadata_id = file_metadata.id "\
                  "AND file.fk_file_category_id=:fk_file_category_id "\
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
    int fieldCopyright = query.record().indexOf("copyright");

    while (query.next()) {
        if (!insertTitle(query.value(fieldId).toInt(),
                         query.value(fieldGenre).toString(),
                         query.value(fieldArtist).toString(),
                         query.value(fieldTitle).toString(),
                         query.value(fieldDuration).toInt(),
                         query.value(fieldTrackNumber).toInt(),
                         query.value(fieldYear).toInt(),
                         query.value(fieldComment).toString(),
                         query.value(fieldHasId3Picture).toBool(),
                         query.value(fieldCopyright).toString()))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertTitle(int fileId, const QString & genre,
                                 const QString & artist, const QString & title,
                                 int duration, int trackNumber, int year,
                                 const QString & comment, bool hasId3Picture,
                                 const QString & copyright)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("INSERT INTO music_title(fk_file_id, fk_music_genre_id, "\
                       "                        fk_music_artist_id, title, duration, "\
                       "                        track_number, year, comment, has_id3_picture,"\
                       "                        copyright) "\
                       "VALUES("\
                       "  :fileId, "\
                       "  (SELECT id FROM music_genre where name=:genre),"\
                       "  (SELECT id FROM music_artist where name=:artist),"\
                       "  :title, :duration, :track_number, :year, :comment,"\
                       "  :has_id3_picture, :copyright"
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
    query.bindValue(":copyright", copyright);
    if (!query.exec())
    {
        // uncomment for DEBUG if needed
        //NDatabase::debugLastQuery("insertTitle failed", query);
        return true;
    }

    return true;
}

void NMusicDatabase::createAlbumTitleTable()
{
    QSqlQuery query(*m_db);

    // TODO: delete entries when no more used
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS music_album_title (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "fk_music_title_id INTEGER UNIQUE NOT NULL," \
            "fk_music_album_id INTEGER NOT NULL" \
            ")"))
        NDatabase::debugLastQuery("music_album_title table creation failed", query);

    if (!query.exec("CREATE INDEX IF NOT EXISTS idx_music_album_title_index "\
                    "ON music_album_title(fk_music_album_id, fk_music_title_id)"))
        NDatabase::debugLastQuery("idx_music_album_title_index creation failed", query);

    if (!query.exec(
            "CREATE TRIGGER IF NOT EXISTS delete_music_album_title " \
            "BEFORE DELETE ON music_title "\
            "FOR EACH ROW BEGIN "\
            "  DELETE from music_album_title WHERE fk_music_title_id = OLD.id; "\
            "END; "))
        NDatabase::debugLastQuery("file CREATE TRIGGER delete_music_album_title failed", query);
}

bool NMusicDatabase::updateAlbumTitleTable()
{
    NDB.beginTransaction();

    if (!populateAlbumTitle())
    {
        NDB.abortTransaction();
        return false;
    }

    NDB.commitTransaction();
    return true;
}

bool NMusicDatabase::populateAlbumTitle()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT music_title.id,file_metadata.album "\
                  "FROM music_title, file, file_metadata "\
                  "WHERE music_title.fk_file_id=file.id "\
                  "AND file.fk_file_metadata_id=file_metadata.id ";

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
    if (!query.prepare("INSERT INTO music_album_title(fk_music_title_id, fk_music_album_id) "\
                       "VALUES("\
                       "  :titleId, "\
                       "  (SELECT id FROM music_album where name=:album)"\
                       ")"))
    {
        NDatabase::debugLastQuery("insertAlbumTitle prepare failed", query);
        return false;
    }

    query.bindValue(":titleId", titleId);
    query.bindValue(":album", albumName);
    if (!query.exec())
    {
        // uncomment for DEBUG if needed
        //NDatabase::debugLastQuery("insertAlbumTitle failed", query);
        return true;
    }

    return true;
}


bool NMusicDatabase::updateAlbumCover()
{
    NDB.beginTransaction();

    // for each album
    // for each album title
    // looking title with id3_picture
    // looking for picture in title directory
    /*if (!populateAlbumCover())
    {
        NDB.abortTransaction();
        return false;
    }*/

    NDB.commitTransaction();
    return true;
}

/*bool NMusicDatabase::populateAlbumCover()
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT music_album.id "\
                  "FROM music_album ";

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("populateAlbumCover prepare failed", query);
        return false;
    }

    if (!query.exec())
    {
        NDatabase::debugLastQuery("populateAlbumCover failed", query);
        return false;
    }

    int fieldId = query.record().indexOf("id");


    while (query.next()) {
        if (!insertAlbumCover(query.value(fieldId).toInt()))
            return false;
    }

    return true;
}

bool NMusicDatabase::insertAlbumCover(int albumId)
{
    QSqlQuery query(*m_db);
    if (!query.prepare("INSERT INTO music_album_cover(fk_music_album_id) "\
                       "VALUES(:albumId"\
                       ")"))
    {
        NDatabase::debugLastQuery("insertAlbumCover prepare failed", query);
        return false;
    }

    query.bindValue(":albumId", albumId);
    if (!query.exec())
    {
        // uncomment for DEBUG if needed
        //NDatabase::debugLastQuery("insertAlbumCover failed", query);
        return true;
    }

    return true;
}
*/

bool NMusicDatabase::getAlbumList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                                  const QStringList & searches, int start, int limit,
                                  const QString & dir, int year, const QString & genre,
                                  const QString & artist)
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT music_album.name album, music_album.front_cover_picture_file_hash fcpfh, "\
                  "music_album.back_cover_picture_file_hash bcpfh, "\
                  "music_album.front_cover_id3picture_file_hash fcipfh, "\
                  "music_album.back_cover_id3picture_file_hash bcipfh "\
                  "FROM music_album, music_album_title, music_title, music_genre, music_artist "\
                  "WHERE music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id "\
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_title.fk_music_genre_id = music_genre.id ";

    if (year >= 0)
        sql += "AND music_title.year = :year ";
    if (!genre.isNull())
        sql += "AND music_genre.name = :genre ";
    if (!artist.isNull())
        sql += "AND music_artist.name = :artist ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("GROUP BY album ORDER BY album %2 LIMIT :limit OFFSET :offset").
           arg(NDatabase::stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getAlbumList prepare failed", query);
        return false;
    }

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
        NDatabase::debugLastQuery("getAlbumList failed", query);
        return false;
    }

    int fieldAlbum = query.record().indexOf("album");
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
        svAlbum.setProperty("album", QScriptValue("album-all"));
        svAlbum.setProperty("frontCoverPictureFileHash", "");
        svAlbum.setProperty("backCoverPictureFileHash", "");
        svAlbum.setProperty("frontCoverID3PictureFileHash", "");
        svAlbum.setProperty("backCoverID3PictureFileHash", "");
    }

    while (query.next()) {
        QScriptValue svAlbum = se.newObject();
        dataArray.setProperty(i, svAlbum);
        svAlbum.setProperty("album", query.value(fieldAlbum).toString());
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
        svAlbum.setProperty("album", QScriptValue("album-all"));
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

    QString sql = "SELECT count(DISTINCT music_album.name) "\
                  "FROM music_album, music_album_title, music_title, music_genre, music_artist "\
                  "WHERE music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id "\
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_title.fk_music_genre_id = music_genre.id ";

    if (year >= 0)
        sql += "AND music_title.year = :year ";
    if (!genre.isNull())
        sql += "AND music_genre.name = :genre ";
    if (!artist.isNull())
        sql += "AND music_artist.name = :artist ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
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
        NDatabase::debugLastQuery("getAlbumListCount failed", query);
        return 0;
    }

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

    QString sql = "SELECT music_artist.name artist "\
                  "FROM music_artist, music_album, music_album_title, music_title, music_genre "\
                  "WHERE music_artist.id = music_title.fk_music_artist_id "\
                  "AND music_title.fk_music_genre_id = music_genre.id "
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id ";


    if (year >= 0)
        sql += "AND music_title.year = :year ";
    if (!genre.isNull())
        sql += "AND music_genre.name = :genre ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("GROUP BY music_artist.name ORDER BY music_artist.name %2 LIMIT :limit OFFSET :offset").
           arg(NDatabase::stringToSortDirection(dir));


    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getArtistList prepare failed", query);
        return false;
    }

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
        NDatabase::debugLastQuery("getArtistList failed", query);
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

int NMusicDatabase::getArtistListCount(const QStringList & searches, int year, const QString & genre)
{
    QSqlQuery query(*m_db);

    QString sql = "SELECT count(DISTINCT music_artist.name) "\
                  "FROM music_artist, music_album, music_album_title, music_title, music_genre "\
                  "WHERE music_artist.id = music_title.fk_music_artist_id "\
                  "AND music_title.fk_music_genre_id = music_genre.id "
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id ";

    if (year >= 0)
        sql += "AND music_title.year = :year ";
    if (!genre.isNull())
        sql += "AND music_genre.name = :genre ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
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

    QString sql = "SELECT music_genre.name genre "\
                  "FROM music_genre, music_artist, music_album, music_album_title, music_title "\
                  "WHERE music_genre.id = music_title.fk_music_genre_id "
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id ";

    if (year >= 0)
        sql += "AND music_title.year = :year ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("GROUP BY music_genre.name ORDER BY genre %2 LIMIT :limit OFFSET :offset").
           arg(NDatabase::stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getGenreList prepare failed", query);
        return false;
    }

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
        NDatabase::debugLastQuery("getGenreList failed", query);
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

int NMusicDatabase::getGenreListCount(const QStringList & searches, int year)
{
    QSqlQuery query(*m_db);


    QString sql = "SELECT count(DISTINCT music_genre.name) "\
                  "FROM music_genre, music_artist, music_album, music_album_title, music_title "\
                  "WHERE music_genre.id = music_title.fk_music_genre_id "
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id ";

    if (year >= 0)
        sql += "AND music_title.year = :year ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
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
                  "FROM music_title, music_genre, music_artist, music_album, music_album_title "\
                  "WHERE year IS NOT NULL "
                  "AND music_genre.id = music_title.fk_music_genre_id "
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id ";


    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_title.year = :year%1 ").arg(i);
        sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString("GROUP BY year ORDER BY year %2 LIMIT :limit OFFSET :offset").
           arg(NDatabase::stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getYearList prepare failed", query);
        return false;
    }


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
                  "FROM music_title, music_genre, music_artist, music_album, music_album_title "\
                  "WHERE year IS NOT NULL "
                  "AND music_genre.id = music_title.fk_music_genre_id "
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id ";


    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_title.year = :year%1 ").arg(i);
        sql += QString("OR music_genre.name LIKE :genre%1 ").arg(i);
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
        query.bindValue(QString(":year%1").arg(i), QString("%%1%").arg(searches.at(i)));
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
    QString sql = "SELECT music_title.id id, file.file_name, file.hash, file.size, "\
                  "       music_artist.name artist, music_title.comment, music_title.year, music_album.name album, "\
                  "       music_title.title, music_genre.name genre, music_title.track_number, music_title.duration, "\
                  "       music_title.copyright, music_title.has_id3_picture "\
                  "FROM music_title, music_genre, music_artist, music_album, music_album_title, file "\
                  "WHERE music_genre.id = music_title.fk_music_genre_id "\
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id "\
                  "AND file.id = music_title.fk_file_id ";

    if (year >= 0)
        sql += "AND music_title.year = :year ";
    if (!genre.isNull())
        sql += "AND music_genre.name = :genre ";
    if (!artist.isNull())
        sql += "AND music_artist.name = :artist ";
    if (!album.isNull())
        sql += "AND music_album.name = :album ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    // Sort and limit
    sql += QString(" GROUP BY music_title.id ORDER BY %1 %2 LIMIT :limit OFFSET :offset").
           arg(NDatabase::jsFileStringToDBFileField(sort)).
           arg(NDatabase::stringToSortDirection(dir));

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getTitleList prepare failed", query);
        return false;
    }

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
        NDatabase::debugLastQuery("getTitleList failed", query);
        return false;
    } else {
        NLOGD("NMusicDatabase", query.lastQuery());
    }

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
    int fieldCopyright = query.record().indexOf("copyright");
    int hasID3Picture = query.record().indexOf("has_id3_picture");


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

int NMusicDatabase::getTitleListCount(const QStringList & searches, const QString & album,
                                      const QString & artist, const QString & genre,
                                      int year)
{
    QSqlQuery query(*m_db);
     QString sql = "SELECT count(music_title.id) "\
                  "FROM music_title, music_genre, music_artist, music_album, music_album_title, file "\
                  "WHERE music_genre.id = music_title.fk_music_genre_id "\
                  "AND music_title.fk_music_artist_id = music_artist.id "\
                  "AND music_album.id = music_album_title.fk_music_album_id "\
                  "AND music_album_title.fk_music_title_id = music_title.id "\
                  "AND file.id = music_title.fk_file_id ";

    if (year >= 0)
        sql += "AND music_title.year = :year ";
    if (!genre.isNull())
        sql += "AND music_genre.name = :genre ";
    if (!artist.isNull())
        sql += "AND music_artist.name = :artist ";
    if (!album.isNull())
        sql += "AND music_album.name = :album ";

    for (int i = 0; i < searches.count(); ++i){
        sql += QString("AND (music_genre.name LIKE :genre%1 ").arg(i);
        sql += QString("OR music_artist.name LIKE :artist%1 ").arg(i);
        sql += QString("OR music_album.name LIKE :album%1 ").arg(i);
        sql += QString("OR music_title.title LIKE :title%1) ").arg(i);
    }

    if (!query.prepare(sql))
    {
        NDatabase::debugLastQuery("getTitleListCount prepare failed", query);
        return 0;
    }

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
        NDatabase::debugLastQuery("getTitleListCount failed", query);
        return 0;
    }else {
        NLOGD("NMusicDatabase", query.lastQuery());
    }

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}
