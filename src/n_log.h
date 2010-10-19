/*
 *n_log.h - log class
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

#ifndef N_LOG_H
#define N_LOG_H

// Qt
#include <QString>
#include <QStringList>
#include <QThread>

class NLog: public QObject
{ 
    Q_OBJECT

public:
    enum LogType {
        ltDebug,
        ltMsg
    };
    static void start();
    static void log(const QString & context, const QString & msg, NLog::LogType lt = ltMsg);
    static void log(const QString & context, const QStringList & msgList, NLog::LogType lt = ltMsg);
    static void directLog(const QString & context, const QString & msg, NLog::LogType lt = ltMsg);
    static void directLog(const QString & context, const QStringList & msgList, NLog::LogType lt = ltMsg);

signals:
    void logString(const QString & context, const QString & msg, NLog::LogType lt);
    void logList(const QString & context, const QStringList & msgList, NLog::LogType lt);
    void directLogString(const QString & context, const QString & msg, NLog::LogType lt);
    void directLogList(const QString & context, const QStringList & msgList, NLog::LogType lt);

private:
    class NLogThread: public QThread
    {
    public:
        NLogThread(NLog * log)
            :m_log(log){
            Q_ASSERT(m_log != NULL);
        }

    protected:
        void run();

    private:
        NLog *m_log;
    };

    static NLog *m_instance;
    NLogThread  *m_logThread;

    NLog(QObject *parent = 0);
    ~NLog();
    static NLog & instance();
    static void deleteInstance();

    void pLog(const QString & context, const QString & msg, NLog::LogType lt);
    void pLog(const QString & context, const QStringList & msgList, NLog::LogType lt);
    void pDirectLog(const QString & context, const QString & msg, NLog::LogType lt);
    void pDirectLog(const QString & context, const QStringList & msgList, NLog::LogType lt);
};

class NLogger: public QObject
{
    Q_OBJECT
public slots:
    void log(const QString & context, const QString & msg, NLog::LogType lt);
    void log(const QString & context, const QStringList & msgList, NLog::LogType lt);

protected:
    virtual void writeDebug(const QString & context, const QString & msg);
    virtual void writeMessage(const QString & context, const QString & msg);

private:
    //syslog date format
    static const QString now();
    QString formatMsg(const QString & msg);
};



inline void logMessage(const QString & context, const QString & msg) {
    NLog::log(context, msg, NLog::ltMsg);
}

inline void logMessageDirect(const QString & context, const QString & msg) {
    NLog::directLog(context, msg, NLog::ltMsg);
}

inline void logDebug(const QString & context, const QString & msg) {
#ifdef DEBUG
    NLog::log(context, msg, NLog::ltDebug);
#endif //DEBUG
}

inline void logDebug(const QString & context, const QStringList & msgList) {
#ifdef DEBUG
    NLog::log(context, msgList, NLog::ltDebug);
#endif //DEBUG
}
inline void logDebugDirect(const QString & context, const QString & msg) {
#ifdef DEBUG
    NLog::directLog(context, msg, NLog::ltDebug);
#endif //DEBUG
}

#endif // N_LOG_H
