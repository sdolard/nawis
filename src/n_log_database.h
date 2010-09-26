/*
 * n_log_database.h - log database class
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


#ifndef N_LOG_DATABASE_H
#define N_LOG_DATABASE_H

// Qt
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMutex>
#include <QFileInfo>
#include <QStringList>
#include <QScriptEngine>
#include <QScriptValue>

// App
#include "n_string_map.h"

#define NLOGDB (NLogDatabase::instance())

class NLogDatabase
{
public:
    static NLogDatabase & instance();
    static void deleteInstance();
    ~NLogDatabase();

    QString lastDbError();

    // log
    bool addLog(const QString & log);
    bool getLogList(QScriptEngine & se, QScriptValue & dataArray, const QString & search,
                    int start = 0, int limit = 25, const QString & sort = "",
                    const QString & dir = "");
    int getLogListCount(const QString & search);
    bool clearLogs();

    static QDateTime toDateTime(const QString & datetTime);

private:
    static NLogDatabase   *m_instance;
    QSqlDatabase         m_db;
    QMutex               m_dbMutex;
    NLogDatabase();
    void create();
    void debugLastQuery(const QString & msg, const QSqlQuery & query);
    QString & addAND(QString & sql, bool *AND);
    void debug(const QString & context, const QString & msg);
    QString stringToLogField(const QString & field);
};

#endif //N_LOG_DATABASE_H
