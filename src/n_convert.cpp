/*
 * n_convert.cpp - convertion methods
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

#include "n_convert.h"

// Qt
#include <QTime>


const QString unitByteToHuman(qint64 unit, const QString & unitString, qint64 bytes)
{
    if (unit <= 0)
        return "--";

    qint64 remainder = bytes % unit;
    if (remainder > 0)
    {
        QString decimal = QString::number(((double) remainder / (double) unit) + 0.05).left(3);
        if (decimal[0].digitValue() == 1)
            return QString("%1%2").arg(bytes / unit + 1).arg(unitString);

        decimal.remove(0, 2);
        if (decimal.at(0).digitValue() == 0)
            return QString("%1%2").arg(bytes / unit).arg(unitString);
        return QString("%1.%2%3").arg(bytes / unit).arg(decimal).arg(unitString);
    }
    return QString("%1%2").arg(bytes / unit).arg(unitString);

}	

const QString NConvert_n::byteToHuman(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = 1024 * KB;
    const qint64 GB = 1024 * MB;
    const qint64 TB = 1024 * GB;

    if (bytes < KB) // less than 1 KB
        return QString("%1B").arg(bytes);

    if (bytes < MB) // less than 1 MB
        return unitByteToHuman(KB, "KB", bytes);

    if (bytes < GB) // less than 1 GB
        return unitByteToHuman(MB, "MB", bytes);

    if (bytes < TB) // less than 1 TB
        return unitByteToHuman(GB, "GB", bytes);

    return unitByteToHuman(TB, "TB", bytes);

}

const QString NConvert_n::durationToHuman(qint64 ms)
{	
    const qint64 OneSec = 1000;
    const qint64 OneMin = OneSec * 60;
    const qint64 OneHour = OneMin * 60;
    const qint64 OneDay = OneHour * 24;
    qint64 z, s, m, h, d;
    d = ms / OneDay;
    h = ms % OneDay / OneHour;
    m = ms % OneHour / OneMin;
    s = ms % OneMin / OneSec;
    z = ms % OneSec;
    QString result;
    if (d)
        result = QString("%1d ").arg(d);
    if (h)
        result += QString("%1h ").arg(h);
    if (m)
        result += QString("%1m ").arg(m);
    if (s)
        result += QString("%1s ").arg(s);
    if (z)
        result += QString("%1ms ").arg(z);

    if (result.isEmpty())
        return "0ms";

    return result.trimmed();
}

const QString NConvert_n::rateForOneSecToHuman(qint64 bytes, qint64 ms)
{
    if (ms <= 0)
        return QString("%1/ms").arg(byteToHuman(1000 * bytes));

    return QString("%1/s").arg(byteToHuman(1000 * bytes / ms));
}

const QStringList NConvert_n::fromUTF8PercentEncoding(const QStringList & list)
{
    QStringList convert;
    for(int i = 0; i < list.count(); ++i)
        convert.append(NConvert_n::fromUTF8PercentEncoding(list.at(i)));
    return convert;
}

const QString NConvert_n::fromUTF8PercentEncoding(const QString & string)
{
    QByteArray data;
    data.append(string);
    return QString::fromUtf8(QByteArray::fromPercentEncoding(data));
}
