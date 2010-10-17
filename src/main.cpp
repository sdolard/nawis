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
#include <QMetaType>
#include <QSslSocket>
#include <QSettings>


// Applicaton 
#include "n_version.h"
#include "n_log.h"
#include "n_nawis_daemon.h"
#include "n_dir_watcher.h"

int main(int argc, char *argv[])
{	
#if QT_VERSION < 0x040400
    You_need_at_least_Qt_4_4_to_compile_this_program
#endif

    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    qRegisterMetaType<QSslSocket::SslMode>("QSslSocket::SslMode");
    qRegisterMetaType<QSslError>("QSslError");
    qRegisterMetaType< QList<QSslError> >("QList<QSslError>");
    qRegisterMetaType<NDirWatcherThreadItems>("NDirWatcherThreadItems");
    qRegisterMetaType<NLog::LogType>("NLog::LogType");

    NDaemon daemon(argc, argv);
    return daemon.exec();
};
