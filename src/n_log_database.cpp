/*
 * n_log_database.cpp - log database class
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
#include <QApplication>
#include <QSqlError>
#include <QMutexLocker>
#include <QDateTime>
#include <QSqlRecord>

// App
#include "n_path.h"
#include "n_config.h"
#include "n_convert.h"
#include "n_log.h"
#include "n_database.h"

#include "n_log_database.h"


NLogDatabase * NLogDatabase::m_instance = NULL;

NLogDatabase & NLogDatabase::instance()
{
    if (m_instance == NULL)
        m_instance = new NLogDatabase();
    return *m_instance;
}

void NLogDatabase::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NLogDatabase::NLogDatabase()
{
    QMutexLocker locker(&m_dbMutex);

    m_db = QSqlDatabase::addDatabase("QSQLITE", "LOG");

    m_db.setDatabaseName(QString("%1%2_log.dbg").arg(NPath_n::config()).
                         arg(qApp->applicationName()));

    if (!m_db.open())
    {
        qDebug("NLogDatabase: m_db.open() failed %s", qPrintable(m_db.lastError().text()));
        Q_ASSERT(false);
        return;
    }
    create();
}

NLogDatabase::~NLogDatabase()
{
    m_db.close();
}

void NLogDatabase::create()
{
    QSqlQuery query(m_db);
    /**
    * Database creation
    */
    // log table creation
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS logs (" \
            "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL," \
            "log TEXT NOT NULL," \
            "date TIMESTAMP NOT NULL" \
            ")"))
        debugLastQuery("logs table creation failed", query);

    // category index creation
    if (!query.exec(
            "CREATE INDEX IF NOT EXISTS idx_logs_date ON logs(date)"))
        debugLastQuery("idx_logs_date creation failed", query);
}


void NLogDatabase::debugLastQuery(const QString & msg, const QSqlQuery & query)
{
    debug("NLogDatabase", "Data base error:");
    debug("NLogDatabase", QString("  %1").arg(msg));
    debug("NLogDatabase", QString("  %1").arg(query.lastError().text()));
    debug("NLogDatabase", QString("  %1").arg(query.lastQuery()));
}

QString NLogDatabase::lastDbError()
{
    return m_db.lastError().text();
}

QString & NLogDatabase::addAND(QString & sql, bool *AND)
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

bool NLogDatabase::addLog(const QString & log)
{
    if (log.isEmpty())
        return false;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO logs (log, date) "\
                  "VALUES(:log, :date)");
    query.bindValue(":log", log);
    query.bindValue(":date", QDateTime::currentDateTime().toString(SQLITE_DATETIME));
    if (!query.exec())
    {
        if (query.lastError().number() != SQLITE_CONSTRAINT)
            debugLastQuery("addLog failed", query);
        return false;
    }
    return true;
}

bool NLogDatabase::getLogList(QScriptEngine & se, QScriptValue & dataArray,
                              const QString & search, int start, int limit,
                              const QString & sort, const QString & dir)
{
    QSqlQuery query(m_db);
    bool useAND = false; // so use WHERE
    QString sql = "SELECT id, date, log " \
                  "FROM logs ";

    // TODO: update this stuped split
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    for (int i = 0; i < searches.count(); ++i)
    {
        addAND(sql, &useAND);
        QString s = searches.at(i);
        sql += QString("log LIKE :search%1 ").arg(i);
    }

    // Sort and limit
    sql += QString("ORDER BY %1 %2 LIMIT :limit OFFSET :start").
           arg(stringToLogField(sort)).
           arg(NDatabase::stringToSortDirection(dir));


    query.prepare(sql);
    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":search%1").arg(i), QString("%%%1%%").arg(s));
    }

    query.bindValue(":limit", limit);
    query.bindValue(":start", start);
    if (!query.exec())
    {
        debugLastQuery("logList failed", query);
        return false;
    }

    int fieldDate = query.record().indexOf("date");
    int fieldLog = query.record().indexOf("log");
    int fieldId = query.record().indexOf("id");
    int i = 0;
    while (query.next()) {
        QScriptValue svLog = se.newObject();
        dataArray.setProperty(i, svLog);

        // Files field
        svLog.setProperty("id", query.value(fieldId).toInt());
        svLog.setProperty("date", query.value(fieldDate).toString());
        svLog.setProperty("log", query.value(fieldLog).toString());
        i++;
    }
    return true;
}

int NLogDatabase::getLogListCount(const QString & search)
{
    QSqlQuery query(m_db);
    bool useAND = false; // so use WHERE
    QString sql = "SELECT COUNT(*) " \
                  "FROM logs ";

    // TODO: update this stuped split
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    for (int i = 0; i < searches.count(); ++i)
    {
        addAND(sql, &useAND);
        QString s = searches.at(i);
        sql += QString("log LIKE :search%1 ").arg(i);
    }

    query.prepare(sql);
    for (int i = 0; i < searches.count(); ++i)
    {
        QString s = searches.at(i);
        query.bindValue(QString(":search%1").arg(i), QString("%%%1%%").arg(s));
    }

    if (!query.exec())
    {
        debugLastQuery("logListCount failed", query);
        return 0;
    }

    if (!query.first())
        return 0;
    return query.value(0).toInt();
}


bool NLogDatabase::clearLogs()
{
    QString sql = QString("DELETE FROM logs");
    QSqlQuery query(m_db);
    if (!query.exec(sql))
    {
        debugLastQuery("clearLogs failed", query);
        return false;
    }
    return true;
}

QDateTime NLogDatabase::toDateTime(const QString & dateTime)
{
    return QDateTime::fromString(dateTime, SQLITE_DATETIME);
}

void NLogDatabase::debug(const QString & context, const QString & msg)
{
    QString rTrimmedMsg = msg;
    if (msg.endsWith("\r\n"))
        rTrimmedMsg = rTrimmedMsg.left(msg.length() - 2);

    // prompt
    qDebug("%s (debug) %s: %s",
           qPrintable(QDateTime::currentDateTime().toString()),
           qPrintable(context),
           qPrintable(rTrimmedMsg));
}

QString NLogDatabase::stringToLogField(const QString & field)
{
    if (field.isEmpty() ||
        (field != "id" &&
         field != "log"&&
         field != "date")
        )
        return "date";

    return field;
}

