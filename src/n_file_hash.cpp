/*
 * n_file_hash.cpp - file hash class
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
#include <QTime>

// App
#include "n_convert.h"

#include "n_file_hash.h"

#define READ_SIZE      (1014 * 4) // 4ko

NFileHash::NFileHash(const QFileInfo & fi, QCryptographicHash::Algorithm algo)
{
    m_fi = fi;
    m_algo = algo;
}

const QString NFileHash::hash()
{
    QCryptographicHash hash(m_algo);
    QFile file(m_fi.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return "";

#ifdef DEBUG
    QTime t;
    t.start();
#endif
    while (!file.atEnd())
        hash.addData(file.read(READ_SIZE));

#ifdef DEBUG
    qDebug("Full hash for %s: %s (%s for %s: %s)",
           qPrintable(m_fi.fileName()),
           //qPrintable(m_fi.absoluteFilePath()),
           qPrintable(QString(hash.result().toHex())),
           qPrintable(NConvert_n::durationToHuman(t.elapsed())),
           qPrintable(NConvert_n::byteToHuman(m_fi.size())),
           qPrintable(NConvert_n::rateForOneSecToHuman(m_fi.size(), t.elapsed()))
           );
#endif
    return hash.result().toHex();
}
