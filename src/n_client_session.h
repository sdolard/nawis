/*
 * n_client_session.h - Client session class
 * mink
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

#ifndef N_CLIENT_SESSION_H
#define N_CLIENT_SESSION_H

// Qt
#include <QHttpRequestHeader>
#include <QByteArray>
#include <QAbstractSocket>
#include <QUrl>
#include <QString>

// APP
#include "n_compress.h"
#include "n_string_map.h"
#include "n_services.h"

class NClientSession
{
public:
    NClientSession(const QHttpRequestHeader & request, const QByteArray & content,
                   QAbstractSocket *socket, bool ssl);
    NClientSession(const NClientSession & session);

    const QHttpRequestHeader & request() const;
    const QByteArray         & content() const;
    const QAbstractSocket    * socket() const;
    const QUrl               & url() const;

    // Return true if session is coming from localhost (ipv6 ready)
    bool isLocal() const;

    // Peer address
    const QString peerAddress() const;

    // Return service associated to url
    NService_n::NService getHTTPMethodService() const;

    void getServices(NService_n::NService* services) const;

    // Return last URL path
    const QString resource() const;

    // Return true if we can compress (gzip or deflate)
    bool supportCompression(NCompress_n::CompressionType ct) const;

    // Return post content to a NStringMap
    const NStringMap contentToMap() const;

    const QString host() const;

    int port() const;

    const QString urlPath() const;

    const QString sessionId() const;

    const QString userAgent() const;

    bool isInternetExplorer() const;

    const QString contentTypeCharset() const;

    bool isSsl() const;

private:
    QHttpRequestHeader m_request;
    QByteArray         m_content;
    QAbstractSocket   *m_socket;
    QUrl               m_url;
    QString            m_sessionId;
    QString            m_userAgent;
    bool               m_isInternetExplorer;
    bool               m_ssl;

    const QStringList paths() const;
    const QString getSessionId() const;
};

#endif //N_CLIENT_SESSION_H
