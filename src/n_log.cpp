/*
 * n_log.cpp - log class
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
// std
#include <iostream> 

// Qt
#include <QMutexLocker>
#include <QDateTime>
#include <QtGlobal>
#include <QPair>
#include <QHostInfo>
#include <QThread>
#include <QApplication>

#include "n_log_database.h"

#include "n_log.h"

NLog * NLog::m_instance = NULL;

void NLog::start()
{
	instance();
}

NLog & NLog::instance()
{
	if (m_instance == NULL)
		m_instance = new NLog();

	return *m_instance;
}

void NLog::deleteInstance()
{
	if (m_instance == NULL)
		return;
	delete m_instance;
 	m_instance = NULL;
}

NLog::NLog(QObject *)
{
	m_logThread = new NLogThread(this);
	m_logThread->start();
}

NLog::~NLog()
{
	m_logThread->exit();
	delete m_logThread;
}

void NLog::log(const QString & context, const QString & msg, NLog::LogType lt)
{
	instance().pLog(context, msg, lt);
}

void NLog::log(const QString & context, const QStringList & msgList, NLog::LogType lt)
{
	instance().pLog(context, msgList, lt);
}

void NLog::pLog(const QString & context, const QString & msg, NLog::LogType lt)
{
	emit logString(context, msg, lt);
}

void NLog::pLog(const QString & context, const QStringList & msgList, NLog::LogType lt)
{
	emit logList(context, msgList, lt);
}

void NLog::directLog(const QString & context, const QString & msg, NLog::LogType lt)
{
	instance().pDirectLog(context, msg, lt);
}

void NLog::directLog(const QString & context, const QStringList & msgList, NLog::LogType lt)
{
	instance().pDirectLog(context, msgList, lt);
}

void NLog::pDirectLog(const QString & context, const QString & msg, NLog::LogType lt)
{
	emit directLogString(context, msg, lt);
}

void NLog::pDirectLog(const QString & context, const QStringList & msgList, NLog::LogType lt)
{
	emit directLogList(context, msgList, lt);
}


/*******************************************************************************
* NLogger
*******************************************************************************/
void NLogger::log(const QString & context, const QString & msg, NLog::LogType lt)
{
	switch (lt)
	{
 case NLog::ltDebug:
		debug(context, msg);
		break;

 case NLog::ltMsg:
		message(context, msg);
		break;
	}
}

void NLogger::log(const QString & context, const QStringList & msgList, NLog::LogType lt)
{
	foreach(QString msg, msgList)
		log(context, msg, lt);
}

void NLogger::debug(const QString & context, const QString & msg)
{
	QString rTrimmedMsg = msg;
	if (msg.endsWith("\r\n"))
		rTrimmedMsg = rTrimmedMsg.left(msg.length() - 2);

	// prompt
	QString prompt = QString("(debug) %1: %2").arg(context).arg(rTrimmedMsg);
	qDebug("%s %s", qPrintable(now()), qPrintable(prompt));
}

void NLogger::message(const QString & context, const QString & msg)
{
	QString rTrimmedMsg = msg;
	if (msg.endsWith("\r\n"))
		rTrimmedMsg = rTrimmedMsg.left(msg.length() - 2);

	// prompt
	QString prompt = QString("%1: %2").arg(context).arg(rTrimmedMsg);
	std::cout << qPrintable(prompt) << std::endl;

	// database
	NLOGDB.addLog(prompt);
}

const QString NLogger::now()
{
	QString month;
	switch(QDateTime::currentDateTime().date().month())
	{
 case 1: month = "Jan"; break;
 case 2: month = "Feb"; break;
 case 3: month = "Mar"; break;
 case 4: month = "Apr"; break;
 case 5: month = "May"; break;
 case 6: month = "Jun"; break;
 case 7: month = "Jul"; break;
 case 8: month = "Aug"; break;
 case 9: month = "Sep"; break;
 case 10: month = "Oct"; break;
 case 11: month = "Nov"; break;
 case 12: month = "Dec"; break;
 }

	return QString("%1 %2").arg(month).arg(QDateTime::currentDateTime().toString("dd hh:mm:ss"));
}

/*******************************************************************************
* NLogThread
*******************************************************************************/
void NLogThread::run()
{
	NLogger logger;
	connect(m_log, SIGNAL(logString(const QString &, const QString &, NLog::LogType)),
			&logger, SLOT(log(const QString &, const QString &, NLog::LogType)),
			Qt::QueuedConnection);
	connect(m_log, SIGNAL(logList(const QString &, const QStringList &, NLog::LogType)),
			&logger, SLOT(log(const QString &, const QStringList &, NLog::LogType)),
			Qt::QueuedConnection);
	connect(m_log, SIGNAL(directLogString(const QString &, const QString &, NLog::LogType)),
			&logger, SLOT(log(const QString &, const QString &, NLog::LogType)), Qt::DirectConnection);
	connect(m_log, SIGNAL(directLogList(const QString &, const QStringList &, NLog::LogType)),
			&logger, SLOT(log(const QString &, const QStringList &, NLog::LogType)), Qt::DirectConnection);
	exec();
}
