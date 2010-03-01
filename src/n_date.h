/*
 * n_date.h - date methods
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

#ifndef N_DATE_H
#define N_DATE_H

//Qt
#include <QString>
#include <QDateTime>

#define SQLITE_DATE      "yyyy-MM-dd"
#define SQLITE_TIME      "hh:mm:ss.zzz"
#define SQLITE_DATETIME  SQLITE_DATE" "SQLITE_TIME


namespace NDate_n
{
    const QString toHTMLDateTime(const QDateTime & datetime);
    QDateTime fromSQLiteDateTime(const QString & dateTime);
};

#endif // N_DATE_N
