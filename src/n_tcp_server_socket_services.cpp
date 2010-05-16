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
#include "n_stream_writer.h"
#include "n_json.h"
#include "n_file_category.h"

#include "n_tcp_server_socket_services.h"

// See http://www.w3schools.com/XML/xml_xsl.asp for formating xml in browser

#define RSP_SUCCESS                    "success"
#define RSP_MSG                        "message"
#define RSP_MSG_LOADED                 "Data loaded"
#define RSP_MSG_NO_RESULTS             "No results"
#define RSP_MSG_N_DELETED              "%1 deleted"
#define RSP_MSG_N_UPDATED              "Data %1 updated"
#define RSP_MSG_INVALID_JSON           "invalid JSON"
#define RSP_MSG_INVALID_INDEX          "invalid index"
#define RSP_MSG_INVALID_INDEX_PROPERTY "invalid index property"
#define RSP_MSG_ERROR_OCCURRED         "An error occured"
#define RSP_COUNT                      "totalcount"
#define RSP_DATA                       "data"
#define RSP_RESULT                     "result"


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
        return svcGetServiceHelp(services, response);
    }

    switch (service.id)
    {
    case SVC_API:
        return svcGetFullServiceHelp(NService_n::nsAPIServices, response);

    case SVC_API_CFG:
        return svcGetFullServiceHelp(NService_n::nsAPICfgServices, response);

    case SVC_API_CFG_SHARED_DIR:
        return svcSharedDir(session, response);

    case SVC_API_FILE:
        return svcGetFullServiceHelp(NService_n::nsAPIFileServices, response);

    case SVC_API_DOWNLOAD:
        return svcGetFileDownload(statusCode, session, response);

    case SVC_API_DUPLICATED:
        return svcGetFullServiceHelp(NService_n::nsAPIDuplicatedServices, response);

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
        return svcLog(session, response);

    case SVC_API_MUSIC:
        return svcGetFullServiceHelp(NService_n::nsAPIMusicServices, response);

    case SVC_API_MUSIC_ALBUM:
        return svcGetMusicAlbum(session, response);

    case SVC_API_MUSIC_ARTIST:
        return svcGetMusicArtist(session, response);

    case SVC_API_MUSIC_GENRE:
        return svcGetMusicGenre(session, response);

    case SVC_API_MUSIC_ID3_PICTURE:
        return svcGetMusicID3Picture(statusCode, session, response);

    case SVC_API_MUSIC_TITLE:
        return svcGetMusicTitle(session, response);

    case SVC_API_MUSIC_YEAR:
        return svcGetMusicYear(session, response);

    case SVC_API_NOP:
        return svcGetNop(response);

    case SVC_API_AUTH:
        return svcAuth(session, response);
        // TODO: user managment
        /*case SVC_API_USER:
		return svcUser(session, response);*/

    case SVC_FAVICON:
        return svcGetFavicon(response);

    case SVC_HELP:
        return svcGetFullServiceHelp(NService_n::nsServices, response);

    case SVC_API_SEARCH:
        return svcGetFullServiceHelp(NService_n::nsAPISearchServices, response);

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


NResponse & NTcpServerSocketServices::svcGetFullServiceHelp(NService_n::NService* rootServices, NResponse & response)
{
    int size = 0;
    NService_n::NService services[999];
    NService_n::getAllServices(rootServices, services, &size);
    return svcGetServiceHelp(services, response);
}

NResponse & NTcpServerSocketServices::svcGetServiceHelp(NService_n::NService* services, NResponse & response)
{
    Q_ASSERT(services);

    QString htmlHelpTemplate = "<html><head><title>%1</title>"\
                               "</head><body>%2</body></html>";
    htmlHelpTemplate = htmlHelpTemplate.arg(QString("nawis server api %1 %2").arg(NAWIS_VERSION).arg(NAWIS_BUILD));
    QString htmlCommandTemplate = "<h2>Command: %1</h2>";
    QString servicesHelp;
    int i = 0;
    while (true)
    {
        if (services[i].id == SVC_NONE)
            break;
        servicesHelp += htmlCommandTemplate.arg(services[i].fullService);
        servicesHelp += QString("<p>%1<br><b>Appears in</b>: %2<br>"\
                                "<b>Authentication required</b>: %3<br>"\
                                "<b>Required level</b>: \"%4\"<br>"\
                                "<b>HTTP method</b>: %5<br>"\
                                "<b>URL params</b>:<br> %6<br>"\
                                "<b>Post data</b>: %7<br>"\
                                "<b>Return</b>: %8</p>").
                arg(services[i].comment).
                arg(services[i].history).
                arg(services[i].authRequired ? "yes" : "no").
                arg(NTcpServerAuthSession::levelToString(services[i].requiredLevel)).
                arg(services[i].httpMethod).
                arg(services[i].params.isEmpty() ? "none": services[i].params).
                arg(services[i].postData.isEmpty() ? "none": services[i].postData).
                arg(services[i].returns);

        servicesHelp += "<hr />";
        ++i;
    }
    response.setData(htmlHelpTemplate.arg(servicesHelp).toUtf8());
    response.httpHeader().setContentType("text/html");
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
                                   arg(NCONFIG.isSslServer() ? "s" : "").
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
    //NLOGD("path", path);

    path.remove(0, 3); // We remove wanted "/ui" begin path chars
    if (!path.isEmpty())
    {
        filename = path;
        uiPath = filename;
    }
    //NLOGD("uiPath", uiPath);

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
                                       arg(NCONFIG.isSslServer() ? "s" : "").
                                       arg(session.host()).
                                       arg(session.port()).
                                       arg(uiPath));
        response.add10yExpiresHttpHeader();
        return response;
    }

    QString fileSuffix = fi.suffix();

    // Redirection to ui/index.hlml if not authenticated
    //NLOGD("fileSuffix", fileSuffix);
    //NLOGD("nawis_sessionid", session.sessionId());
    if (fileSuffix == "html" &&
        uiPath != "/index.html" &&
        !m_authSessionHash.isValid(session))
    {
        return svcRedirectToTUI(statusCode, session, response);
    }

    QString gzippedFilename = filename + ".gz";
    QFileInfo gzipFi(gzippedFilename);
    //NLOGD("NTcpServerSocketServices::svcUI filename", filename);
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

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
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

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
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

NResponse & NTcpServerSocketServices::svcLog(const NClientSession & session, NResponse & response)
{	
    if (session.request().method() == "GET")
        return svcGetLog(session, response);

    if (session.request().method() == "DELETE")
        return svcDeleteLog(response);
    Q_ASSERT(false);
    return response;
}

NResponse & NTcpServerSocketServices::svcDeleteLog(NResponse & response)
{	
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    bool logCleared = NLOGDB.clearLogs();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, logCleared ? "Log cleared" : "Log not cleared");
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetLog(const NClientSession & session,
                                                NResponse & response)
{	
    bool ok;
    QString search = session.url().queryItemValue("search");
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString sort = session.url().queryItemValue("sort");
    QString dir = session.url().queryItemValue("dir");

    int totalCount = NLOGDB.getLogListCount(search);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NLOGDB.getLogList(se, svData, search, start, limit, sort, dir);
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
    /*NLOGD("NTcpServerSocketServices", QString("setResponseStatus: %1").
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

NResponse & NTcpServerSocketServices::svcSharedDir(const NClientSession & session, NResponse & response)
{
    if (session.request().method() == "POST")
        return svcPostSharedDir(session, response);
    if (session.request().method() == "PUT")
        return svcPutSharedDir(session, response);
    if (session.request().method() == "GET")
        return svcGetSharedDir(session, response);
    if (session.request().method() == "DELETE")
        return svcDeleteSharedDir(session, response);

    Q_ASSERT(false);
    return response;
}

// Configuration
NResponse & NTcpServerSocketServices::svcGetSharedDir(const NClientSession &, NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    NDirList sd = NCONFIG.sharedDirectories();
    int count = sd.count();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(count > 0));
    svRoot.setProperty(RSP_MSG, QScriptValue(count > 0 ? RSP_MSG_LOADED : RSP_MSG_NO_RESULTS));
    svRoot.setProperty(RSP_COUNT, QScriptValue(count));

    QScriptValue svData = se.newArray(count);
    svRoot.setProperty(RSP_DATA, svData);
    for(int i = 0; i < count; i++)
    {
        NDir dir = sd.at(i);
        QScriptValue svDir = se.newObject();
        svData.setProperty(i, svDir);
        svDir.setProperty("id", i);
        svDir.setProperty("path", dir.path());
        svDir.setProperty("recursive", dir.recursive());
        svDir.setProperty("shared", dir.shared());
        svDir.setProperty("exists", dir.exists());
    }
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcPostSharedDir(const NClientSession & session, NResponse & response)
{		
    //NLOGD("NTcpServerSocketServices::svcPostSharedDir", session.postData());

    QScriptEngine se;
    QScriptValue svReadData = se.evaluate("data = " + session.postData());
    svReadData = se.toObject(svReadData);
    NDir dir = NDir(svReadData.property("path").toString(),
                    svReadData.property("recursive").toBool(),
                    svReadData.property("shared").toBool());

    int id  = NCONFIG.addSharedDirectory(dir);

    QScriptValue svRoot = se.newObject();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_LOADED));
    QScriptValue svData = se.newArray(1);
    svRoot.setProperty(RSP_DATA, svData);
    QScriptValue svDir = se.newObject();
    svData.setProperty(0, svDir);
    svDir.setProperty("id", id);
    svDir.setProperty("path", dir.path());
    svDir.setProperty("recursive", dir.recursive());
    svDir.setProperty("shared", dir.shared());
    svDir.setProperty("exists", dir.exists());
    //NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcPutSharedDir(const NClientSession & session, NResponse & response)
{
    //NLOGD("NTcpServerSocketServices::svcPutSharedDir", session.postData());
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    QScriptValue svReadData = se.evaluate("data = " + session.postData());
    if (se.hasUncaughtException()){
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_JSON));
        NLOGD("NTcpServerSocketServices::svcPutSharedDir", se.uncaughtExceptionBacktrace());
        NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    QScriptValue svReadDir = svReadData.property("data").toObject();
    QString strId = svReadDir.property("id").toString();
    NLOGD("svcPutSharedDir strId", strId);
    bool ok;
    int id = strId.toInt(&ok);
    if (!ok)
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX_PROPERTY));
        //NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    NDirList sharedDirs = NCONFIG.sharedDirectories();
    if (id >= sharedDirs.count())
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX));
        //NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    QString path = svReadDir.property("path").toString();
    QString recursive = svReadDir.property("recursive").toString();
    QString shared = svReadDir.property("shared").toString();

    NLOGD("strId", strId);
    NLOGD("path", path);
    NLOGD("recursive", recursive);
    NLOGD("shared", shared);

    NDir dir = sharedDirs.at(id);
    NDir modifiedDir = NDir(path.isEmpty() ? dir.path() : path,
                            recursive.isEmpty() ? dir.recursive() : QVariant(recursive).toBool(),
                            shared.isEmpty() ? dir.shared() : QVariant(shared).toBool());

    modifiedDir = NCONFIG.modifySharedDirectory(id, modifiedDir);
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue(QString(RSP_MSG_N_UPDATED).arg(id)));
    QScriptValue svData = se.newArray();
    svRoot.setProperty(RSP_DATA, svData);
    QScriptValue svDir = se.newObject();
    svData.setProperty(0, svDir);
    svDir.setProperty("id", id);
    svDir.setProperty("path", modifiedDir.path());
    svDir.setProperty("recursive", modifiedDir.recursive());
    svDir.setProperty("shared", modifiedDir.shared());
    svDir.setProperty("exists", modifiedDir.exists());
    //NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcDeleteSharedDir(const NClientSession & session, NResponse & response)
{
    NLOGD("NTcpServerSocketServices::svcDeleteSharedDir", session.postData());
    int id = 0;
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QString strId = session.resource();
    bool ok;
    id = strId.toInt(&ok);
    if (!ok)
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX));
        //NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    NDirList sharedDirs = NCONFIG.sharedDirectories();
    if (id >= sharedDirs.count())
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX));
        //NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }
    NCONFIG.removeSharedDirectory(id);

    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue(QString(RSP_MSG_N_DELETED).arg(id)));
    //NLOGD("NJson::serialize(svRoot)", NJson::serialize(svRoot));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

void NTcpServerSocketServices::removeExpiredSession()
{
    m_authSessionHash.removeExpired();
}

NResponse & NTcpServerSocketServices::svcAuth(const NClientSession & session, NResponse & response)
{
    if (session.request().method() == "POST")
        return svcPostAuth(session, response);

    if (session.request().method() == "DELETE")
        return svcDeleteAuth(session, response);

    if (session.request().method() == "GET")
    {
        // POST, GET, DELETE, PUT, END > five elements in array
        NService_n::NService services[5];
        session.getServices(services);
        return svcGetServiceHelp(services, response);
    }

    Q_ASSERT(false);
    return response;
}

NResponse & NTcpServerSocketServices::svcPostAuth(const NClientSession & session, NResponse & response)
{
    QScriptEngine se;
    QString login;
    QString pwd;
    if (session.request().contentType() == NMimeType_n::fileSuffixToMIME("json")){
        QScriptValue svReadRoot = se.newObject();
        QScriptValue svReadData = se.evaluate("data = " + session.postData());
        if (se.hasUncaughtException()){
            svReadRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
            svReadRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_JSON));
            NLOGD("NTcpServerSocketServices::svcPostAuths", se.uncaughtExceptionBacktrace());
            NLOGD("NJson::serialize(svRoot)", NJson::serialize(svReadRoot));
            response.setData(NJson::serializeToQByteArray(svReadRoot));
            return response;
        }
        svReadData = se.toObject(svReadData);
        login = svReadData.property("username").toString();
        pwd = svReadData.property("password").toString();
    }

    if (session.request().contentType() == NMimeType_n::fileSuffixToMIME("form"))
    {
        const NStringMap data = session.postDataToMap();
        login = data["username"];
        pwd = data["password"];
    }


    int level = AUTH_LEVEL_USER;
    QScriptValue svRoot = se.newObject();

    // TODO: implement non admin auth

    // Admin auth
    if (NCONFIG.AdminUser() == login &&
        NCONFIG.AdminPassword() == pwd)
    {
        level = level | AUTH_LEVEL_ADMIN;

        // Some one try to auth many time on the same IP
        // Other browser, IP nat ? So access is granted, but we log
        QString sessionId = m_authSessionHash.sessionId(session.peerAddress(), login);
        if (!sessionId.isEmpty())
        {
            const NTcpServerAuthSession & authSession = m_authSessionHash.value(sessionId);
            NLOGM("WARNING: Authentication login while already logged in",
                  QString("From %1@%2; Already logged from %3; level: %4; user agent: %5").
                  arg(login).
                  arg(session.peerAddress()).
                  arg(authSession.address()).
                  arg(NTcpServerAuthSession::levelToString(authSession.level())).
                  arg(session.userAgent()));
        }

        // Administrator level
        NTcpServerAuthSession authSession;
        authSession.set(session.peerAddress(), login, session.userAgent(), level);
        m_authSessionHash.insert(authSession.sessionId(), authSession);
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
        svRoot.setProperty(RSP_MSG, QScriptValue("Authentication succeed"));
        svRoot.setProperty("level", QScriptValue(NTcpServerAuthSession::levelToString(authSession.level())));
        response.setSessionCookie(authSession.sessionId());
        NLOGM("Authentication login succeed", QString("%1@%2; level: %3; user agent: %4").
              arg(authSession.login()).
              arg(authSession.address()).
              arg(NTcpServerAuthSession::levelToString(authSession.level())).
              arg(authSession.userAgent()));
    } else {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue("Authentication failed"));

        NLOGM("Authentication login failed", QString("%1@%2; user agent: %3").
              arg(login).
              arg(session.peerAddress()).
              arg(session.userAgent()));
    }
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcDeleteAuth(const NClientSession & session, NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    if (m_authSessionHash.contains(session.sessionId()))
    {
        const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
        if (authSession.address() == session.peerAddress())
        {
            NLOGM("Authentication logout succeed", QString("%1@%2; user agent: %3").
                  arg(authSession.login()).
                  arg(session.peerAddress()).
                  arg(session.userAgent()));
            svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
            svRoot.setProperty(RSP_MSG, QScriptValue(QString("%1 unauthenticated").arg(authSession.login())));
            m_authSessionHash.remove(session.sessionId());
            response.clearSessionCookie();
        } else {
            NLOGM("Authentication logout WARNING",
                  QString("try to unauth %1@%2 (authentication address is %3); user agent: %4").
                  arg(authSession.login()).
                  arg(session.peerAddress()).
                  arg(authSession.address()).
                  arg(session.userAgent()));

            svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
            svRoot.setProperty(RSP_MSG, QScriptValue(QString("Try to unauthenticate %1 from a different "\
                                                             "authenticated address. Your Ip is logged.(%2)").
                                                     arg(authSession.login()).
                                                     arg(session.peerAddress())));

        }
    } else {
        NLOGM("Authentication logout ERROR", QString("no authentication for %1; user agent: %2").
              arg(session.peerAddress()).
              arg(session.userAgent()));
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue("You are not authenticated"));
    }
    response.setData(NJson::serializeToQByteArray(svRoot));
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

NResponse & NTcpServerSocketServices::svcUser(const NClientSession & session, NResponse & response)
{
    if (session.request().method() == "GET")
        return svcGetUser(session, response);

    if (session.request().method() == "POST")
        return svcPostUser(session, response);

    if (session.request().method() == "DELETE")
        return svcDeleteUser(session, response);

    Q_ASSERT(false);
    return response;
}

NResponse & NTcpServerSocketServices::svcGetUser(const NClientSession & session, NResponse & response)
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

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
          tr("%1 is looking for users: \"%2\"; start: %3; limit: %4, sort:\"%5\", dir:\"%6\"").
         arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(sort).arg(dir));

    int totalCount = NDB.getUserListCount(searches);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NDB.getUserList(se, svData, searches, start, limit, sort, dir);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcPostUser(const NClientSession & session, NResponse & response)
{
    NStringMap params = session.postDataToMap();
    QString firstName = params.value("firstName");
    NLOGD("svcUserRegister firstName", firstName);
    QString lastName = params.value("lastName");
    QString email = params.value("email");
    QString password = params.value("password");

    NStringMap user = NDB.getUser(email);
    KsStreamWriter sw(&response.data(), session);

    if (user.count() == 0)
    {
        if (NDB.registerUser(firstName, lastName, email, password))
        {
            sw.writeTokenBool(RSP_SUCCESS, true);
            sw.writeToken(RSP_MSG, QString("User %1 registered").arg(email));
            NLOGM("Registration succeed", QString("%1(%2); user agent: %3").
                  arg(email).
                  arg(session.peerAddress()).
                  arg(session.userAgent()));
        } else {
            sw.writeTokenBool(RSP_SUCCESS, false);
            sw.writeToken(RSP_MSG, "Unknown reason");
            NLOGM("Registration failed", QString("unknown error; %1(%2); user agent: %3").
                  arg(email).
                  arg(session.peerAddress()).
                  arg(session.userAgent()));
        }
    } else {
        sw.writeTokenBool(RSP_SUCCESS, false);
        sw.writeToken(RSP_MSG, "Already registered");
        NLOGM("Registration failed", QString("already_registered; %1(%2); user agent: %3").
              arg(email).
              arg(session.peerAddress()).
              arg(session.userAgent()));
        //TODO:  Did he lost his password ?
    }
    return response;
}

NResponse & NTcpServerSocketServices::svcDeleteUser(const NClientSession & session, NResponse & response)
{	
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QString id = session.resource();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(NDB.deleteUser(id)));
    svRoot.setProperty(RSP_MSG, QScriptValue(QString("User %1 deleted").arg(id)));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetMusicAlbum(const NClientSession & session,
                                                       NResponse & response)
{	
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    QString artist;
    if(session.url().hasQueryItem("artist"))// Cos of NULL test
    {
        artist = session.url().queryItemValue("artist");
        if (artist.isNull())
            artist = "";
        artist = NConvert_n::fromUTF8PercentEncoding(artist);
    }
    QString genre;
    if(session.url().hasQueryItem("genre"))// Cos of NULL test
    {
        genre = session.url().queryItemValue("genre");
        if (genre.isNull())
            genre = "";
        genre = NConvert_n::fromUTF8PercentEncoding(genre);
    }

    int year = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
          tr("%1 is looking for album: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));

    int totalCount = NMDB.getAlbumListCount(searches, year, genre, artist);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    // TODO: manage limit = -1 to get all album
    bool succeed = NMDB.getAlbumList(se, svData, totalCount, searches, start,
                                         limit, dir, year, genre, artist);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetMusicArtist(const NClientSession & session,
                                                        NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    int year  = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    QString genre;
    if(session.url().hasQueryItem("genre"))// Cos of NULL test
    {
        genre = session.url().queryItemValue("genre");
        if (genre.isNull())
            genre = "";
        genre = NConvert_n::fromUTF8PercentEncoding(genre);
    }

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
          tr("%1 is looking for artist: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));

    int totalCount = NMDB.getArtistListCount(searches, year, genre);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NMDB.getArtistList(se, svData, totalCount, searches, start, limit, dir, year, genre);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetMusicGenre(const NClientSession & session,
                                                       NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    int year  = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
          tr("%1 is looking for genre: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));

    int totalCount = NMDB.getGenreListCount(searches, year);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NMDB.getGenreList(se, svData, totalCount, searches, start, limit, dir, year);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetMusicID3Picture(int *statusCode,
                                                            const NClientSession & session,
                                                            NResponse & response)
{
    response.removeDefaultCharset();

    QString fileHash = session.resource();
    QFileInfo fileInfo = NDatabase::instance().file(fileHash);

    // Cache
    QString lastModified = NDate_n::toHTMLDateTime(fileInfo.lastModified());
    response.httpHeader().setValue("Last-Modified", lastModified);
    if (session.request().value("If-Modified-Since") == lastModified)
    {
        *statusCode = N_HTTP_NOT_MODIFIED;
        return response;
    }

    // File
    QByteArray ba;
    QString mimeType;
    if (NMetadata::getID3Picture(fileInfo.absoluteFilePath(), ba, mimeType))
    {
        response.setData(ba);
        response.httpHeader().setContentType(mimeType);
        response.add10yExpiresHttpHeader();
    }

    return response;
}

NResponse & NTcpServerSocketServices::svcGetMusicYear(const NClientSession & session,
                                                      NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
          tr("%1 is looking for year: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));


    int totalCount = NMDB.getYearListCount(searches);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);

    bool succeed = NMDB.getYearList(se, svData, totalCount, searches, start,
                                        limit, dir);
    setJsonRootReponse(svRoot, totalCount, succeed);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketServices::svcGetMusicTitle(const NClientSession & session,
                                                       NResponse & response)
{
    bool ok;

    int start = session.url().queryItemValue("start").toInt();
    if (!ok)
        start = 0;
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    QString sort = session.url().queryItemValue("sort");
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);

    QString album;
    if(session.url().hasQueryItem("album")) // Cos of NULL test
    {
        album = session.url().queryItemValue("album");
        if (album.isNull())
            album = "";
        album = NConvert_n::fromUTF8PercentEncoding(album);

    }
    QString artist;
    if(session.url().hasQueryItem("artist"))// Cos of NULL test
    {
        artist = session.url().queryItemValue("artist");
        if (artist.isNull())
            artist = "";
        artist = NConvert_n::fromUTF8PercentEncoding(artist);
    }
    QString genre;
    if(session.url().hasQueryItem("genre"))// Cos of NULL test
    {	genre = session.url().queryItemValue("genre");
        if (genre.isNull())
            genre = "";
        genre = NConvert_n::fromUTF8PercentEncoding(genre);
    }

    int year  = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
    NLOGM(session.socket()->peerAddress().toString(),
          tr("%1 is looking for music search:\"%2\"; album:\"%3\"; artist:\"%4\";"\
             "genre:\"%5\"; year:\"%6\"; start:\"%7\"; "\
             "limit:\"%8\"; sort:\"%9\"; dir:\"%10\"").
          arg(authSession.login()). // 1
          arg(NConvert_n::fromUTF8PercentEncoding(search)).// 2
          arg(album).// 3
          arg(artist).// 4
          arg(genre).// 5
          arg(year).// 6
          arg(start).// 7
          arg(limit).// 8
          arg(sort).// 9
          arg(dir));// 10

    int totalCount = NMDB.getTitleListCount(searches, album, artist, genre, year);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);

    bool succeed = NMDB.getTitleList(se, svData, searches, album, artist, genre, year,
                                         start, limit, sort, dir);

    setJsonRootReponse(svRoot, totalCount, succeed);

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
        !m_authSessionHash.isValid(session, service.requiredLevel))
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
