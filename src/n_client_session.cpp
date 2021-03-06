/*
 * n_client_session.cpp - Client session class
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

#include <QPair>
#include <QHostAddress>
#include <QList>
#include <QNetworkCookie>

#include "n_log.h"
#include "n_config.h"

#include "n_client_session.h"


NClientSession::NClientSession(const QHttpRequestHeader & request,
                               const QByteArray & content, QAbstractSocket *socket,
                               bool ssl)
                                   :m_request(request), m_content(content),
                                   m_pSocket(socket), m_ssl(ssl)
{
    Q_ASSERT(socket);

    QByteArray url;
    url.append(m_ssl ? "https://" : "http://");
    url.append(request.value("host"));
    url.append(request.path());
    m_url = QUrl(url);
    Q_ASSERT(m_url.isValid());
    m_sessionId = getSessionId();

    m_userAgent = m_request.value("User-Agent");
    m_isInternetExplorer = m_userAgent.contains("MSIE");
}

NClientSession::NClientSession(const NClientSession & session)
{
    this->m_request = session.m_request;
    this->m_content = session.m_content;
    this->m_pSocket = session.m_pSocket;
    this->m_url = session.m_url;
    this->m_sessionId = session.m_sessionId;
    this->m_userAgent = session.m_userAgent;
    this->m_isInternetExplorer = session.m_isInternetExplorer;
    this->m_ssl = session.m_ssl;
}

const QString NClientSession::getSessionId() const {
    // TODO: put session id in URL?
    QStringList cookieList = QString(m_request.value("Cookie")).split("; ", QString::SkipEmptyParts);
    for(int i = 0; i < cookieList.count(); ++i)
    {
        //logDebug("getSessionId(), cookiePair", cookieList.at(i));
        QStringList cookie = cookieList.at(i).split("=");
        if (cookie.count() > 2 || cookie.count() == 0)
            return "";
        if (cookie[0] == "nawis_sessionId")
        {
            if (cookie.count() == 2)
                return cookie[1];
            return "";
        }
    }
    return "";
}

const QString NClientSession::sessionId() const
{
    return m_sessionId;
}

const QHttpRequestHeader & NClientSession::request() const
{
    return m_request;
}

const QUrl & NClientSession::url() const
{
    return m_url;
}

const QByteArray & NClientSession::content() const
{
    return m_content;
}

const QAbstractSocket * NClientSession::socket() const
{
    return m_pSocket;
}

bool NClientSession::isLocal() const
{
    return (m_pSocket->peerAddress() == QHostAddress::LocalHost) ||
            (m_pSocket->peerAddress() == QHostAddress::LocalHostIPv6);
}

const QString NClientSession::peerAddress() const
{
    return m_pSocket->peerAddress().toString();
}

const QStringList NClientSession::paths() const
{
    QStringList list = m_url.path().split("/");
    list.removeFirst();
    if (list.last().isEmpty())
        list.removeLast();
    return list;
}

const QString NClientSession::resource() const
{
    return m_url.path().split("/").last();
}

bool NClientSession::supportCompression(NCompress_n::CompressionType ct) const
{
    // Accept-Encoding gzip,deflate
    QStringList list = m_request.value("Accept-Encoding").split(",", QString::SkipEmptyParts);
    QStringList trimmedList;
    for(int i = 0; i < list.count(); i++)
        trimmedList.append(list[i].trimmed());

    return trimmedList.contains(NCompress_n::toString(ct), Qt::CaseInsensitive);
}

const QString NClientSession::contentTypeCharset() const
{
    if (!m_request.hasContentType())
        return "";

    QStringList contentTypes = m_request.contentType().split("; ", QString::SkipEmptyParts);
    for(int i = 0; i< contentTypes.count(); i++)
    {
        if (contentTypes[i].compare("charset=UTF-8", Qt::CaseInsensitive))
            return "UTF8";
    }
    return "";
}

const NStringMap NClientSession::contentToMap() const
{
    QStringList stringList;
    if (contentTypeCharset() == "UTF8")
        stringList = QString::fromUtf8(m_content).split("&");
    else
        stringList = QString(m_content).split("&");
    NStringMap postParamList;
    for(int i = 0; i < stringList.count(); i++)
    {
        QStringList data = stringList[i].split("=");
        if (data.count() ==  0)
            continue;
        if (data.count() ==  1)
        {
            postParamList.insert(data.at(0), "");
            continue;
        }
        postParamList.insert(data.at(0), data.at(1));
    }
    return postParamList;
}

const QString NClientSession::host() const
{
    return m_url.host();
}

int NClientSession::port() const
{
    return m_url.port();
}

const QString NClientSession::urlPath() const
{
    return m_url.path();
}

NService_n::NService NClientSession::getHTTPMethodService() const
{ 
    return NService_n::getHTTPMethodService(NService_n::nsServices, paths(), m_request.method());
}

void NClientSession::getServices(NService_n::NService* services) const
{
    int size = 0;
    NService_n::getServices(NService_n::nsServices, paths(), services, &size);
}

const QString NClientSession::userAgent() const
{
    return m_userAgent;
}

bool NClientSession::isInternetExplorer() const
{
    return m_isInternetExplorer;
}

bool NClientSession::isSsl() const
{
    return m_ssl;
}
