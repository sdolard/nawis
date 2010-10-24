/*
 * n_image.cpp - Image class
 * nawis
 * Created by Sébastien Dolard on 16/05/09.
 * Copyright 2009. All rights reserved.
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
#include <QImage>
#include <QImageReader>

// exiv2 
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/preview.hpp>

// App
#include "n_mime_type.h"
#include "n_log.h"

#include "n_image.h"

#define MAX_WIDTH  (1024 * 3)
#define MAX_HEIGHT ( 768 * 3)


NImage::NImage(const QFileInfo & fi)
    :m_fi(fi)
{	
    m_defaultThumbSize = QSize(160, 120);
    m_defaultResizeSize = QSize(800, 600);
}


const QByteArray NImage::getThumb()
{	
    if (!m_fi.exists())
        return getNoPreviewImage(m_defaultThumbSize);

    // exiv
    QByteArray ba = getExivThumb();
    if (ba.size() != 0)
        return ba;

    // home made
    ba = pResize(m_defaultThumbSize);
    if (ba.size() != 0)
        return ba;

    // no preview
    return getNoPreviewImage(m_defaultThumbSize);
}

const QByteArray NImage::resize(const QSize & size)
{
    const QSize & s = checkSize(size);

    if (!m_fi.exists())
        return getNoPreviewImage(s);

    // heavy
    QByteArray ba = pResize(s);
    if (ba.size() != 0)
        return ba;

    // no preview
    return getNoPreviewImage(s);
}

const QByteArray NImage::getExivThumb()
{
    QByteArray ba;
    try { //exif thumb
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)QFile::encodeName(m_fi.absoluteFilePath()));
        if (image.get() == 0)
            return ba;

        image->readMetadata();

        Exiv2::ExifThumbC exifThumb(image->exifData());
        Exiv2::DataBuf thumb = exifThumb.copy();
        if (thumb.size_ == 0) {
            //logDebugDirect("NImage::getExivThumb", "failed");
            return ba;
        }
        //logDebugDirect("NImage::getExivThumb", "success");

        QBuffer buffer(&ba);
        buffer.setData((const char*)thumb.pData_,  thumb.size_);
        m_mimeType = exifThumb.mimeType();
    }
    catch (Exiv2::AnyError& e) {
        logDebug("Caught Exiv2 exception", e.what());
        return ba;
    }

    return ba;
}

const QByteArray NImage::getNoPreviewImage(const QSize & size)
{	
    const QSize & s = checkSize(size);

    QImage image(":/images/no_preview.png");

    if( s.width() == 0 )
        image = image.scaledToHeight ( s.height(), Qt::SmoothTransformation );
    else if( s.height() == 0 )
        image = image.scaledToWidth( s.width(), Qt::SmoothTransformation );
    else
        image = image.scaled(s.width(), s.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG"); // writes image into ba in JPG format
    m_mimeType = "image/jpeg";
    return ba;
}

const QByteArray NImage::pResize(const QSize & size)
{
    QByteArray ba;

    // TODO: add this thumb in picture exif ?
    QImageReader imageReader(m_fi.absoluteFilePath());
    if (!imageReader.canRead())
    {
        logMessage("NImage::pResize error",
                   QString("loading preview for: %1\nError:%2").
                   arg(m_fi.absoluteFilePath()).
                   arg(imageReader.errorString()));
        return ba;
    }

    QImage image = imageReader.read();
    if (image.isNull())
        return ba;

    if( size.width() == 0 )
        image = image.scaledToHeight(size.height(), Qt::SmoothTransformation);
    else if(size.height() == 0 )
        image = image.scaledToWidth(size.width(), Qt::SmoothTransformation);
    else
        image = image.scaled(size.width(), size.height(), Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);

    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG"); // writes image into ba in JPG format
    m_mimeType = "image/jpeg";
    return ba;
}

const QSize NImage::checkSize(const QSize & size) const
{
    if (size.isNull() || !size.isValid())
        return m_defaultResizeSize;

    if (size.width() <= MAX_WIDTH &&
        size.height() <= MAX_HEIGHT)
        return size;

    QSize s(size);
    if (s.width() > MAX_WIDTH)
        s.setWidth(MAX_WIDTH);

    if (s.height() > MAX_HEIGHT)
        s.setHeight(MAX_HEIGHT);

    return s;
}

const QString NImage::mimeType() const
{
    return m_mimeType;
}
