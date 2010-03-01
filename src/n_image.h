/*
 *  n_image.h - Image class
 *  nawis
 *  Created by Sébastien Dolard on 16/05/09.
 *  Copyright 2009. All rights reserved.
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
#include <QString>
#include <QFileInfo>
#include <QSize>
#include <QByteArray>
#include <QBuffer>


class NImage {
public: 
    NImage(const QFileInfo & fi); // ctor

    // Return JPG data
    const QByteArray getThumb();
    // Return JPG data
    const QByteArray resize(const QSize & size);

    /*
	 * Only available after preview call
	 * return an empty string other wise 
	 */
    const QString mimeType() const;

private:
    QFileInfo m_fi;
    QSize     m_defaultThumbSize;
    QSize     m_defaultResizeSize;
    QString   m_mimeType;

    // Return JPG data
    const QByteArray notExistsImage(const QSize & size);

    // Return exiv thumbnail
    // If many exist, and returnGreater is set to true, biggest one
    // will be return.
    const QByteArray exivThumb(bool returnGreater = true);

    // Return JPG data
    const QByteArray pResize(const QSize & size);

    const QSize checkSize(const QSize & size) const;
};
