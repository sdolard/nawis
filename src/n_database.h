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

#define SQLITE_OK           0   /* Successful result */
/* beginning-of-error-codes */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_INTERNAL     2   /* Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* NOT USED. Table or record not found */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* NOT USED. Database lock protocol error */
#define SQLITE_EMPTY       16   /* Database is empty */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Auxiliary database format error */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */
/* end-of-error-codes */


#define NDB (NDatabase::instance())

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
    void setUserLevel(const QString & level);
    bool registerUser(const QString & firstName, const QString & lastName,
                      const QString & email, const QString & password);
    const NStringMap getUser(const QString & email);
    bool deleteUser(const QString & id);
    bool requestUserPassord(const QString & email);

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

    QString jsFileStringToDBFileField(const QString & jsString);

    QString stringToUserField(const QString & field);
    bool setDuplicatedFileAsNotDeleted(const QFileInfo & fi);
    bool isDuplicatedFile(const QString & hash, const QFileInfo & newFi);
    bool setFileAsNotDeleted(const QString & absoluteFilePath, const QDateTime & lastModified);
};

#endif //N_DATABASE_H

