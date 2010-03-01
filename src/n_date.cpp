/*
 * n_date.cpp - date methods
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


#include "n_date.h"

const QString NDate_n::toHTMLDateTime(const QDateTime & datetime)
{
    //	Sat, 08 Nov 2008 14:08:39 GMT
    QDateTime UTC = datetime.toUTC();
    QString month;
    switch(UTC.date().month())
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

    QString day;
    switch(UTC.date().dayOfWeek())
    {
    case 1: day = "Mon"; break;
    case 2: day = "Tue"; break;
    case 3: day = "Wed"; break;
    case 4: day = "Thu"; break;
    case 5: day = "Fri"; break;
    case 6: day = "Sat"; break;
    case 7: day = "Sun"; break;
    }

    return day +", " + UTC.toString("dd ") + month + UTC.toString(" yyyy hh:mm:ss") + " GMT";
}	

QDateTime NDate_n::fromSQLiteDateTime(const QString & dateTime)
{
    return QDateTime::fromString(dateTime, SQLITE_DATETIME);
}

