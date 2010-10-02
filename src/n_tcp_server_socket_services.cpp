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
#include <QScriptEngine>

// App
#include "n_http.h"
#include "n_json.h"
#include "n_tcp_server_socket_auth_services.h"
#include "n_tcp_server_socket_log_services.h"
#include "n_tcp_server_socket_shared_dir_services.h"
#include "n_tcp_server_socket_user_services.h"
#include "n_tcp_server_socket_music_services.h"
#include "n_tcp_server_socket_picture_services.h"
#include "n_tcp_server_socket_file_services.h"
#include "n_tcp_server_socket_ui_services.h"

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

NResponse NTcpServerSocketServices::getResponse(const NClientSession & session)
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
        return getUIServices().redirectToTUI(statusCode, session, response);

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
        return getFileServices().download(statusCode, session, response);

    case SVC_API_DUPLICATED:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPIDuplicatedServices, response);

    case SVC_API_DUPLICATED_FILE:
        return getFileServices().getDuplicated(session, response, NFileCategory_n::fcAll);

    case SVC_API_DUPLICATED_ARCHIVE:
        return getFileServices().getDuplicated(session, response, NFileCategory_n::fcArchive);

    case SVC_API_DUPLICATED_DOCUMENT:
        return getFileServices().getDuplicated(session, response, NFileCategory_n::fcDocument);

    case SVC_API_DUPLICATED_MUSIC:
        return getFileServices().getDuplicated(session, response, NFileCategory_n::fcMusic);

    case SVC_API_DUPLICATED_OTHER:
        return getFileServices().getDuplicated(session, response, NFileCategory_n::fcOther);

    case SVC_API_DUPLICATED_PICTURE:
        return getFileServices().getDuplicated(session, response, NFileCategory_n::fcPicture);

    case SVC_API_DUPLICATED_MOVIE:
        return getFileServices().getDuplicated(session, response, NFileCategory_n::fcMovie);

    case SVC_API_FILE_UPDATE_DB:
        return getUIServices().lookForModification(response);

    case SVC_API_PICTURE_RESIZE:
        return getPictureServices().resize(session, response);

    case SVC_API_PICTURE_THUMB:
        return getPictureServices().getThumb(session, response);

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
        return getUIServices().nop(response);

    case SVC_API_AUTH:
        return NTcpServerSocketAuthServices::instance().auth(session, response);

    case SVC_API_USER:
        return getUserServices().user(session, response);

    case SVC_FAVICON:
        return getUIServices().getFavicon(response);

    case SVC_HELP:
        return NTcpServerSocketService::getFullHelp(NService_n::nsServices, response);

    case SVC_API_SEARCH:
        return NTcpServerSocketService::getFullHelp(NService_n::nsAPISearchServices, response);

    case SVC_API_SEARCH_FILE:
        return getFileServices().search(session, response, NFileCategory_n::fcAll);

    case SVC_API_SEARCH_ARCHIVE:
        return getFileServices().search(session, response, NFileCategory_n::fcArchive);

    case SVC_API_SEARCH_DOCUMENT:
        return getFileServices().search(session, response, NFileCategory_n::fcDocument);

    case SVC_API_SEARCH_MUSIC:
        return getFileServices().search(session, response, NFileCategory_n::fcMusic);

    case SVC_API_SEARCH_OTHER:
        return getFileServices().search(session, response, NFileCategory_n::fcOther);

    case SVC_API_SEARCH_PICTURE:
        return getFileServices().search(session, response, NFileCategory_n::fcPicture);

    case SVC_API_SEARCH_MOVIE:
        return getFileServices().search(session, response, NFileCategory_n::fcMovie);

    case SVC_UI:
        return getUIServices().getUI(statusCode, session, response);

    default:
        Q_ASSERT_X(false, "NTcpServerSocketServices",
                   qPrintable(QString("not managed response id %1").arg(service.id)));
        return getUIServices().redirectToTUI(statusCode, session, response);
    }
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
