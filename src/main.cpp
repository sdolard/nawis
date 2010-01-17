/*
 * main.cpp - nawis main source
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
#include <QCoreApplication>
#include <QMetaType>
#include <QSslSocket>

// Std
#include <signal.h>	
#include <stdlib.h>
#include <stdio.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#endif
#ifndef Q_OS_MAC
#include <iostream>
#endif

// Applicaton 
#include "n_version.h"
#include "n_single_app.h"
#include "n_server.h"
#include "n_log.h"
#include "n_log_database.h"

void sigManager(int sig) {
	switch (sig)
	{	
	case SIGINT:  // terminate process interrupt program
#ifndef Q_OS_WIN32
	case SIGQUIT: // create core image quit program
#endif
	case SIGTERM: // terminate process software termination signal	
			NServer::deleteInstance();
		break;
	}
}

int main(int argc, char *argv[])
{	
#if QT_VERSION < 0x040400
	You_need_at_least_Qt_4_4_to_compile_this_program
#endif
	
	KsSingleApplication singleApp;
	if (!singleApp.attach("nawis"))
		return -1;

	signal(SIGINT, sigManager);	
#ifndef Q_OS_WIN32
	signal(SIGQUIT, sigManager);
#endif
	signal(SIGTERM, sigManager);
		
	QCoreApplication app(argc, argv);

	app.setApplicationName("nawis");
	app.setOrganizationDomain("nawis");
	app.setOrganizationName("nawis");
	  
	qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
	qRegisterMetaType<QSslSocket::SslMode>("QSslSocket::SslMode");
	qRegisterMetaType<QSslError>("QSslError");
	qRegisterMetaType< QList<QSslError> >("QList<QSslError>");
	qRegisterMetaType<NDirWatcherThreadItems>("NDirWatcherThreadItems");
	qRegisterMetaType<NLog::LogType>("NLog::LogType");

	NLog::start(); // log thead init
		
	NServer::instance(&app);
	NLOGM("NServer", "--------------- new session ---------------");
	
	if (!NSERVER.start())
	{
		NLOGMD("NServer can not start", NSERVER.errorMessage());
		return -1;
	}
	
	return app.exec();
};
