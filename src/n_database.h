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
#include "n_string_map.h"
#include "n_date.h"
#include "n_metadata.h"

// User db error
#define DB_USER_ERROR_INVALID_PARAMS         -1
#define DB_USER_ERROR_QUERY                  -2

#define NDB (NDatabase::instance())

// TODO: create mutex cos of thread access on public method only
class NDatabase: public QObject {
    Q_OBJECT
public:
    ~NDatabase();

    static NDatabase & instance();
    static void deleteInstance();
    static void debugLastQuery(const QString & msg, const QSqlQuery & query);
    static QString & addAND(QString & sql, bool *AND);
    static QString stringToSortDirection(const QString & dir);


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
    const QFileInfo getFileToHash();
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

    // See DB_USER_ERROR_*
    int addUser(const QString & name, const QString & email, const QString & password);
    const NStringMap getUserById(int id);
    const NStringMap getUserByEmail(const QString & email);
    bool deleteUser(const QString & id);
    bool updateUser(int id, const QString & email, const QString & password,
                    const QString & name, const QString & preferences,
                    bool enabled, const QString & level);

private:
    static NDatabase    *m_instance;
    QSqlDatabase         m_db;
    QMutex               m_dbMutex;
    bool                 m_transactionPending;
    NDatabase();

    void createTables();
    void createCategoryTable();
    void createFileMetadataTable();
    void createFileTable();
    void createDuplicatedFileTable();
    void createUserTable();
    void enableFk();

    QString jsFileStringToDBFileField(const QString & jsString);

    QString stringToUserField(const QString & field);
    bool setDuplicatedFileAsNotDeleted(const QFileInfo & fi);
    bool isDuplicatedFile(const QString & hash, const QFileInfo & newFi);
    bool setFileAsNotDeleted(const QString & absoluteFilePath, const QDateTime & lastModified);
    const QFileInfo getDbFileToHash();
};

#endif //N_DATABASE_H

