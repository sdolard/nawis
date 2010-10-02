/*
 * n_tcp_server_socket_services.cpp - web server services
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
#include <QDateTime>
#include <QTextDocument>
#include <QFile>
#include <QPair>
#include <QHostAddress>
#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QBuffer>

// App
#include "n_http.h"
#include "n_version.h"
#include "n_log.h"
#include "n_database.h"
#include "n_music_database.h"
#include "n_log_database.h"
#include "n_config.h"
#include "n_date.h"
#include "n_file_category.h"
#include "n_convert.h"
#include "n_mime_type.h"
#include "n_image.h"
#include "n_config.h"
#include "n_server.h"
#include "n_json.h"
#include "n_file_category.h"
#include "n_tcp_server_socket_auth_services.h"
#include "n_tcp_server_socket_log_services.h"
#include "n_tcp_server_socket_shared_dir_services.h"
#include "n_tcp_server_socket_user_services.h"
#include "n_tcp_server_socket_music_services.h"

#include "n_tcp_server_socket_services.h"

// See http://www.w3schools.com/XML/xml_xsl.asp for formating xml in browser


NTcpServerSocketServices * NTcpServerSocketServices::m_instance = NULL;

NTcpServerSocketServices & NTcpServerSocketServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketServices();
    return *m_instance;
}

void NTcpServerSocketServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketServices::NTcpServerSocketServices()
{                
}

NTcpServerSocketServices::~NTcpServerSocketServices()
{          
}

NResponse NTcpServerSocketServices::response(const NClientSession & session)
{	
    // TODO: add in path a random path to prevent xss attack ?
    NResponse response;

    if (!session.request().isValid())
        return setResponseStatus(N_HTTP_BAD_REQUEST, response);

    int statusCode;
    setData(&statusCode, response, session);
    setResponseStatus(statusCode, response);

    switch (response.dataType())
    {
    case NResponse::dtPriorBuffered:
        {
            if (!response.isGZipped() && session.supportCompression(NCompress_n::ctDeflate))
                response.deflateData(session.isInternetExplorer());
            response.httpHeader().setValue("Content-Length",
                                           QString::number(response.data().size()));
            break;
        }

    case NResponse::dtFile:
        {
            response.setFileRange(session.request().value("Range"));
            break;
        }
    default:
        Q_ASSERT(false);
        break;
    }

    response.httpHeader().setValue("Connection", "keep-alive");

    // Result
    response.data().prepend(response.httpHeader().toString().toUtf8());
    return response;
}

NResponse & NTcpServerSocketServices::setData(int *statusCode, NResponse & response,
                                              const NClientSession & session)
{
    Q_ASSERT(statusCode);
    *statusCode = N_HTTP_OK;

    NService_n::NService service = session.getHTTPMethodService();

    if (service.id == SVC_NONE)
        return svcRedirectToTUI(statusCode, session, response);

    if (!isServiceAvailable(statusCode, session, service) &&
        !session.url().hasQueryItem("help"))
        return response;

    if (session.url().hasQueryItem("help")){
        *statusCode = N_HTTP_OK;
        // POST, GET, DELETE, PUT, END > five elements in array
        NService_n::NService services[5];
        session.getServices(services);
        return NTcpServerSocketService::getHelp(services, response);
    }

    switch (service.id)
    {
    case SVC_API:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPIServices, response);

    case SVC_API_CFG:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPICfgServices, response);

    case SVC_API_CFG_SHARED_DIR:
        return getSharedDirServices().sharedDir(session, response);

    case SVC_API_FILE:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPIFileServices, response);

    case SVC_API_DOWNLOAD:
        return svcGetFileDownload(statusCode, session, response);

    case SVC_API_DUPLICATED:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPIDuplicatedServices, response);

    case SVC_API_DUPLICATED_FILE:
        return svcGetDuplicatedFile(session, response, NFileCategory_n::fcAll);

    case SVC_API_DUPLICATED_ARCHIVE:
        return svcGetDuplicatedFile(session, response, NFileCategory_n::fcArchive);

    case SVC_API_DUPLICATED_DOCUMENT:
        return svcGetDuplicatedFile(session, response, NFileCategory_n::fcDocument);

    case SVC_API_DUPLICATED_MUSIC:
        return svcGetDuplicatedFile(session, response, NFileCategory_n::fcMusic);

    case SVC_API_DUPLICATED_OTHER:
        return svcGetDuplicatedFile(session, response, NFileCategory_n::fcOther);

    case SVC_API_DUPLICATED_PICTURE:
        return svcGetDuplicatedFile(session, response, NFileCategory_n::fcPicture);

    case SVC_API_DUPLICATED_MOVIE:
        return svcGetDuplicatedFile(session, response, NFileCategory_n::fcMovie);

    case SVC_API_FILE_UPDATE_DB:
        return svcLookForModification(response);

    case SVC_API_PICTURE_RESIZE:
        return svcGetPictureResize(session, response);

    case SVC_API_PICTURE_THUMB:
        return svcGetPictureThumb(session, response);

    case SVC_API_LOG:
        return getLogServices().log(session, response);

    case SVC_API_MUSIC:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPIMusicServices, response);

    case SVC_API_MUSIC_ALBUM:
        return getMusicServices().getAlbum(session, response);

    case SVC_API_MUSIC_ARTIST:
        return getMusicServices().getArtist(session, response);

    case SVC_API_MUSIC_GENRE:
        return getMusicServices().getGenre(session, response);

    case SVC_API_MUSIC_ID3_PICTURE:
        return getMusicServices().getID3Picture(statusCode, session, response);

    case SVC_API_MUSIC_TITLE:
        return getMusicServices().getTitle(session, response);

    case SVC_API_MUSIC_YEAR:
        return getMusicServices().getYear(session, response);

    case SVC_API_NOP:
        return svcGetNop(response);

    case SVC_API_AUTH:
        return NTcpServerSocketAuthServices::instance().auth(session, response);

    case SVC_API_USER:
        return getUserServices().user(session, response);

    case SVC_FAVICON:
        return svcGetFavicon(response);

    case SVC_HELP:
        return NTcpServerSocketService::getFullHelp(NService_n::nsServices, response);

    case SVC_API_SEARCH:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPISearchServices, response);

    case SVC_API_SEARCH_FILE:
        return svcGetSearch(session, response, NFileCategory_n::fcAll);

    case SVC_API_SEARCH_ARCHIVE:
        return svcGetSearch(session, response, NFileCategory_n::fcArchive);

    case SVC_API_SEARCH_DOCUMENT:
        return svcGetSearch(session, response, NFileCategory_n::fcDocument);

    case SVC_API_SEARCH_MUSIC:
        return svcGetSearch(session, response, NFileCategory_n::fcMusic);

    case SVC_API_SEARCH_OTHER:
        return svcGetSearch(session, response, NFileCategory_n::fcOther);

    case SVC_API_SEARCH_PICTURE:
        return svcGetSearch(session, response, NFileCategory_n::fcPicture);

    case SVC_API_SEARCH_MOVIE:
        return svcGetSearch(session, response, NFileCategory_n::fcMovie);

    case SVC_UI:
        return svcGetUI(statusCode, session, response);

    default:
        Q_ASSERT_X(false, "NTcpServerSocketServices",
                   qPrintable(QString("not managed response id %1").arg(service.id)));
        return svcRedirectToTUI(statusCode, session, response);
    }
    return response;
}

NResponse & NTcpServerSocketServices::svcGetFavicon(NResponse & response)
{
    response.httpHeader().setContentType("image/x-icon");
    response.removeDefaultCharset();

    QFile file(":/images/nawis.ico");
    if (!file.open(QIODevice::ReadOnly))
        return response;
    response.setData(file.readAll());
    response.add10yExpiresHttpHeader();
    return response;
}

NResponse & NTcpServerSocketServices::svcRedirectToTUI(int *statusCode,
                                                       const NClientSession & session, NResponse & response)
{
    *statusCode = N_HTTP_MOVED_PERMANENTLY;
    response.httpHeader().setValue("Location", QString("http%1://%2:%3/ui").
                                   arg(session.isSsl() ? "s" : "").
                                   arg(session.host()).
                                   arg(session.port()));
    response.add10yExpiresHttpHeader();
    return response;
}

NResponse & NTcpServerSocketServices::svcGetUI(int *statusCode,
                                               const NClientSession & session, NResponse & response)
{
    QString filename("/index.html");
    QString uiPath = filename;
    QString path = session.urlPath();
    //logDebug("path", path);

    path.remove(0, 3); // We remove wanted "/ui" begin path chars
    if (!path.isEmpty())
    {
        filename = path;
        uiPath = filename;
    }
    //logDebug("uiPath", uiPath);

    // Directory traversal test
    filename.prepend(NCONFIG.serverPub().absolutePath()); // to local file
    QFileInfo fi(filename);
    if (!fi.absoluteFilePath().startsWith(NCONFIG.serverPub().absolutePath()))
    {
        *statusCode = N_HTTP_FORBIDDEN;
        return response;
    }

    // Redirection to index.html file of dir
    if (fi.isDir())
    {
        *statusCode = N_HTTP_MOVED_PERMANENTLY;
        if (uiPath.endsWith("/"))
            uiPath.remove(path.length() - 1, 1);
        response.httpHeader().setValue("Location", QString("http%1://%2:%3/ui%4/index.html").
                                       arg(session.isSsl() ? "s" : "").
                                       arg(session.host()).
                                       arg(session.port()).
                                       arg(uiPath));
        response.add10yExpiresHttpHeader();
        return response;
    }

    QString fileSuffix = fi.suffix();

    // Redirection to ui/index.hlml if not authenticated
    //logDebug("fileSuffix", fileSuffix);
    //logDebug("nawis_sessionid", session.sessionId());
    //logDebug("path", path);
    //logDebug("m_authSessionHash.isValid(session)", QVariant(m_authSessionHash.isValid(session)).toString());
    if (fileSuffix == "html" &&
        uiPath != "/index.html" &&
        !getAuthServices().isSessionValid(session))
    {
        return svcRedirectToTUI(statusCode, session, response);
    }

    QString gzippedFilename = filename + ".gz";
    QFileInfo gzipFi(gzippedFilename);
    //logDebug("NTcpServerSocketServices::svcUI filename", filename);
    if (session.supportCompression(NCompress_n::ctGZip) && gzipFi.exists())
    {
        fi = gzipFi;
        filename = gzippedFilename;
    } else if (!fi.exists()) {
        *statusCode = N_HTTP_NOT_FOUND;
        return response;
    }

    response.httpHeader().setContentType(NMimeType_n::fileSuffixToMIME(fileSuffix));
    response.add10yExpiresHttpHeader();
    response.removeDefaultCharset();

    QString lastModified = NDate_n::toHTMLDateTime(fi.lastModified());
    response.httpHeader().setValue("Last-Modified", lastModified);
    if (session.request().value("If-Modified-Since") == lastModified)
    {
        *statusCode = N_HTTP_NOT_MODIFIED;
        return response;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return response;
    response.setData(file.readAll());
    if (filename == gzippedFilename)
        response.setGZipped();
    return response;
}

NResponse & NTcpServerSocketServices::svcGetSearch(const NClientSession & session,
                                                   NResponse & response,
                                                   NFileCategory_n::FileCategory category)
{	
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString sort = session.url().queryItemValue("sort");
    QString dir = session.url().queryItemValue("dir");

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          tr("%1 is looking for file: \"%2\"; category: \"%3\"; start: %4; limit: %5, sort:\"%6\", dir:\"%7\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(category).arg(start).arg(limit).arg(sort).arg(dir));

    int totalCount = NDB.getFileListCount(searches, category);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);


    bool succeed = NDB.getFileList(se, svData, searches, start, limit, category, sort, dir);

    setJsonRootReponse(svRoot, totalCount, succeed);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetDuplicatedFile(const NClientSession & session,
                                                           NResponse & response,
                                                           NFileCategory_n::FileCategory category)
{		
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString sort = session.url().queryItemValue("sort");
    QString dir = session.url().queryItemValue("dir");

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          tr("%1 is looking for (duplicated) \"%2\"; category: \"%3\"; start: %4; limit: %5, sort:\"%6\", dir:\"%7\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(category).arg(start).arg(limit).arg(sort).arg(dir));

    int totalCount = NDB.getDuplicatedFileListCount(searches, category);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NDB.getDuplicatedFileList(se, svData, searches, start, limit,
                                             category, sort, dir);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetFileDownload(int *statusCode, const NClientSession & session, NResponse & response)
{
    QString fileHash = session.resource();

    QFileInfo fileInfo = NDatabase::instance().file(fileHash);
    if (!fileInfo.exists())
    {
        *statusCode = N_HTTP_NOT_FOUND;
        return response;
    }
    response.setFileInfo(fileInfo);
    response.httpHeader().setContentType(NMimeType_n::fileSuffixToMIME(fileInfo.suffix()));
    response.removeDefaultCharset();
    return response;
}

NResponse & NTcpServerSocketServices::svcGetPictureResize(const NClientSession & session,
                                                          NResponse & response)
{	
    QString fileHash = session.resource();
    QFileInfo fileInfo = NDatabase::instance().file(fileHash);
    QSize size(0, 0);
    if (session.url().hasQueryItem("width"))
        size.setWidth(session.url().queryItemValue("width").toInt());
    if (session.url().hasQueryItem("height"))
        size.setHeight(session.url().queryItemValue("height").toInt());

    NImage image(fileInfo);
    response.setData(image.resize(size));
    response.httpHeader().setContentType(image.mimeType());
    response.removeDefaultCharset();
    return response;
}

NResponse & NTcpServerSocketServices::svcGetPictureThumb(const NClientSession & session,
                                                         NResponse & response)
{
    QString fileHash = session.resource();
    QFileInfo fileInfo = NDatabase::instance().file(fileHash);

    NImage image(fileInfo);
    response.setData(image.getThumb());
    response.httpHeader().setContentType(image.mimeType());
    response.removeDefaultCharset();
    response.add10yExpiresHttpHeader();
    QString lastModified = NDate_n::toHTMLDateTime(fileInfo.lastModified());
    response.httpHeader().setValue("Last-Modified", lastModified);
    return response;
}

NResponse & NTcpServerSocketServices::setResponseStatus(int statusCode, NResponse & response)
{		
    /*logDebug("NTcpServerSocketServices", QString("setResponseStatus: %1").
	 arg(NHttp_n::statusCodeToString(statusCode)));
	 */
    response.httpHeader().setStatusLine(statusCode, NHttp_n::statusCodeToString(statusCode));

    switch(statusCode)// TODO: create real HTML page
    {
    case N_HTTP_BAD_REQUEST:
    case N_HTTP_UNAUTHORIZED:
    case N_HTTP_METHOD_NOT_ALLOWED:
        // TODO: send a html page
        response.httpHeader().setValue("Connection", "close");
        response.httpHeader().setContentType("text/html");
        response.setData(response.httpHeader().toString().toUtf8());
        break;

    case N_HTTP_NOT_FOUND:
        {
            // TODO: send a html page
            QScriptEngine se;
            QScriptValue svRoot = se.newObject();
            svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
            svRoot.setProperty(RSP_MSG, QScriptValue("Not found: you must try 'help' command"));
            response.setData(NJson::serializeToQByteArray(svRoot));
            break;
        }

    case N_HTTP_OK:
    case N_HTTP_NOT_MODIFIED:
    case N_HTTP_MOVED_PERMANENTLY:
        // nothing
        break;

    case N_HTTP_FORBIDDEN:
        {
            // TODO: send a html page
            QScriptEngine se;
            QScriptValue svRoot = se.newObject();
            svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
            svRoot.setProperty(RSP_MSG, QScriptValue("Forbidden"));
            response.setData(NJson::serializeToQByteArray(svRoot));
            break;
        }
    default:
        Q_ASSERT(false);
    }

    return response;
}

NResponse & NTcpServerSocketServices::svcGetNop(NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue("Nop"));
    svRoot.setProperty("status", QScriptValue(NServer::jobToString(NSERVER.jobStatus())));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcLookForModification(NResponse & response)
{
    NCONFIG.clearDirUpdateData();
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue("Server will looking for modification"));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}





void NTcpServerSocketServices::setJsonRootReponse(QScriptValue & svRoot, int totalCount,
                                                  bool succeed)
{
    svRoot.setProperty(RSP_SUCCESS, succeed ? QScriptValue(true): QScriptValue(false));
    svRoot.setProperty(RSP_COUNT, totalCount > 0 && succeed ? QScriptValue(totalCount): QScriptValue(0));
    if (totalCount == 0)
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_NO_RESULTS));
    else if (succeed)
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_LOADED));
    else
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_ERROR_OCCURRED));
}

bool NTcpServerSocketServices::isServiceAvailable(int * statusCode, const NClientSession & session,
                                                  NService_n::NService service)
{
    Q_ASSERT(statusCode);
    if (service.id == SVC_NONE)
    {
        *statusCode = N_HTTP_BAD_REQUEST;
        return false;
    }

    if (service.authRequired &&
        !getAuthServices().isSessionValid(session, service.requiredLevel))
    {
        *statusCode = N_HTTP_UNAUTHORIZED;
        return false;
    }

    Q_ASSERT_X(!service.httpMethod.isEmpty(),
               "NTcpServerSocketServices::isServiceAvailable: no http method defined",
               qPrintable(service.service));
    if (session.request().method() != service.httpMethod)
    {
        *statusCode = N_HTTP_METHOD_NOT_ALLOWED;
        return false;
    }
    return true;
}
