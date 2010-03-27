/*
 * n_database.h - application database
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

#ifndef N_MUSIC_DATABASE_H
#define N_MUSIC_DATABASE_H

// Qt
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMutex>
#include <QFileInfo>
#include <QStringList>
#include <QScriptEngine>
#include <QScriptValue>

// App
#include "n_file_suffix.h"
#include "n_file_category.h"
#include "n_map.h"
#include "n_date.h"
#include "n_metadata.h"

#define NMDB (NMusicDatabase::instance())

class NMusicDatabase: public QObject {
    Q_OBJECT
public:
    static NMusicDatabase & instance(QSqlDatabase *db = NULL);
    static void deleteInstance();
    ~NMusicDatabase();

    // Music album
    bool getAlbumList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                      const QStringList & searches, int start, int limit,
                      const QString & dir, int year, const QString & genre,
                      const QString & artist);
    int getAlbumListCount(const QStringList & searches, int year, const QString & genre,
                          const QString & artist);

    // Music artist
    bool getArtistList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                       const QStringList & searches, int start, int limit,
                       const QString & dir, int year, const QString & genre);
    int getArtistListCount(const QStringList & searches, int year, const QString & genre);

    // Music genre
    bool getGenreList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                      const QStringList & searches, int start, int limit,
                      const QString & dir, int year);
    int getGenreListCount(const QStringList & searches, int year);

    // Music year
    bool getYearList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                     const QStringList & searches, int start, int limit,
                     const QString & dir);
    int getYearListCount(const QStringList & searches);

    // Music title
    bool getTitleList(QScriptEngine & se, QScriptValue & dataArray,
                      const QStringList & searches, const QString & album,
                      const QString & artist, const QString & genre, int year,
                      int start = 0, int limit = 25, const QString & sort = "",
                      const QString & dir = "");
    int getTitleListCount(const QStringList & searches, const QString & album,
                          const QString & artist, const QString & genre,
                          int year);

    void updateDb();

private:
    static NMusicDatabase   *m_instance;
    QSqlDatabase             *m_db;
    NMusicDatabase(QSqlDatabase  *db);

    void createTables();

    // Music
    void createArtistTable();
    void createGenreTable();
    void createAlbumTitleTable();
    void createTitleTable();
    void createAlbumTable();
    void createAlbumCoverTable();

    bool updateAlbumTable();
    bool updateArtistTable();
    bool updateGenreTable();
    bool updateTitleTable();
    bool updateAlbumTitleTable();
    bool updateAlbumCover();

    // album
    bool setAlbumDeleted(const QString albumName = "", bool deleted = true);
    bool populateAlbum();
    bool insertAlbum(const QString & albumName);
    bool removeDeletedAlbum();

    // artist
    bool setArtistDeleted(const QString artistName = "", bool deleted = true);
    bool populateArtist();
    bool insertArtist(const QString & authorName);
    bool removeDeletedArtist();

    // genre
    bool setGenreDeleted(const QString genreName = "", bool deleted = true);
    bool populateGenre();
    bool insertGenre(const QString & genreName);
    bool removeDeletedGenre();

    // title
    bool populateTitle();
    bool insertTitle(int fileId, const QString & genre, const QString & artist,
                     const QString & title, int duration, int trackNumber,
                     int year, const QString & comment, bool hasId3Picture,
                     const QString & copyright);

    // album title
    bool populateAlbumTitle();
    bool insertAlbumTitle(int titleId, const QString & albumId);


    // album cover
    /*bool populateAlbumCover();
    bool insertAlbumCover(int albumId);*/
};

#endif //N_MUSIC_DATABASE_H

