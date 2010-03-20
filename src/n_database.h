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

#ifndef N_DATABASE_H
#define N_DATABASE_H

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

#define NDB (NDatabase::instance())

class NDatabase: public QObject {
    Q_OBJECT
public:
    static NDatabase & instance();
    static void deleteInstance();
    ~NDatabase();

    // transaction
    bool beginTransaction();
    bool commitTransaction();
    bool abortTransaction();

    QString lastDbError();

    qint64 sharedSize();
    qint64 sizeOfDuplicatedFiles();

    // files
    bool getFileList(QScriptEngine & se, QScriptValue & dataArray, const QStringList & searches,
                     int start = 0, int limit = 25, NFileCategory_n::FileCategory fc = NFileCategory_n::fcAll,
                     const QString & sort = "", const QString & dir= "");
    int getFileListCount(const QStringList & searches, NFileCategory_n::FileCategory fc = NFileCategory_n::fcAll);
    QFileInfo file(const QString & fileHash);
    bool setFilesAsDeleted();
    bool removeDeletedFiles();
    bool addFile(const QFileInfo & fi, const NFileSuffix & suffix, const QString & rootPath);
    const QFileInfo fileToHash();
    void setFileHash(const QFileInfo & fi, const QString & hash);

    // file with no metadata
    bool getFileWithNoMetadata(QString & absoluteFilePath, int & fileId);
    bool setMetadata(int fileId, const NMetadata & id3);

    // duplicated files
    bool getDuplicatedFileList(QScriptEngine & se, QScriptValue & dataArray,
                               const QStringList & searches, int start = 0,
                               int limit = 25, NFileCategory_n::FileCategory fc = NFileCategory_n::fcAll,
                               const QString & sort = "", const QString & dir= "");
    int getDuplicatedFileListCount(const QStringList & searches, NFileCategory_n::FileCategory fc = NFileCategory_n::fcAll);
    bool setDuplicatedFilesAsDeleted();
    bool removeDeletedDuplicatedFiles();

    // Users
    bool getUserList(QScriptEngine & se, QScriptValue & dataArray,
                     const QStringList & searches, int start = 0, int limit = 25,
                     const QString & sort = "", const QString & dir= "");
    int getUserListCount(const QStringList & searches);
    void setUserLevel(const QString & level);
    bool registerUser(const QString & firstName, const QString & lastName,
                      const QString & email, const QString & password);
    const NStringMap getUser(const QString & email);
    bool deleteUser(const QString & id);
    bool requestUserPassord(const QString & email);

    // Music album
    bool getMusicAlbumList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                           const QStringList & searches, int start, int limit,
                           const QString & dir, int year, const QString & genre,
                           const QString & artist);
    int getMusicAlbumListCount(const QStringList & searches, int year, const QString & genre,
                               const QString & artist);

    // Music artist
    bool getMusicArtistList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                            const QStringList & searches, int start, int limit,
                            const QString & dir, int year, const QString & genre);
    int getMusicArtistListCount(const QStringList & searches, int year, const QString & genre);

    // Music genre
    bool getMusicGenreList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                           const QStringList & searches, int start, int limit,
                           const QString & dir, int year);
    int getMusicGenreListCount(const QStringList & searches, int year);

    // Music year
    bool getMusicYearList(QScriptEngine & se, QScriptValue & dataArray, int totalCount,
                          const QStringList & searches, int start, int limit,
                          const QString & dir);
    int getMusicYearListCount(const QStringList & searches);

    // Music title
    bool getMusicTitleList(QScriptEngine & se, QScriptValue & dataArray,
                           const QStringList & searches, const QString & album,
                           const QString & artist, const QString & genre, int year,
                           int start = 0, int limit = 25, const QString & sort = "",
                           const QString & dir = "");
    int getMusicTitleListCount(const QStringList & searches, const QString & album,
                               const QString & artist, const QString & genre,
                               int year);

    bool updateMusicAlbumTable();

    static void debugLastQuery(const QString & msg, const QSqlQuery & query);
    static QString & addAND(QString & sql, bool *AND);
    static QString stringToSortDirection(const QString & dir);

private:
    static NDatabase   *m_instance;
    QSqlDatabase         m_db;
    QMutex               m_dbMutex;
    bool                 m_transactionPending;
    NDatabase();

    void createTables();
    void createCategoryTable();
    void createMetadataTable();
    void createFilesTable();
    void createDuplicatedFilesTable();
    void createUsersTable();

    // Music
    void createMusicAlbumTable();
    void createMusicAlbumTitleTable();
    void createMusicTitleTable();
    void createMusicAuthorTable();
    void createMusicGenreTable();
    void createMusicCoverTable();

    QString jsFileStringToDBFileField(const QString & jsString);
    QString stringToUserField(const QString & field);
    bool setDuplicatedFileAsNotDeleted(const QFileInfo & fi);
    bool isDuplicatedFile(const QString & hash, const QFileInfo & newFi);
    bool setFileAsNotDeleted(const QString & absoluteFilePath, const QDateTime & lastModified);

    bool setMusicAlbumDeleted(const QString albumName = "", bool deleted = true);
    bool populateMusicAlbum();
    bool insertMusicAlbum(const QString & albumName);
    bool removeDeletedMusicAlbum();
};

#endif //N_DATABASE_H

