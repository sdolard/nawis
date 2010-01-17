/*
 * n_tcp_server_socket_services.h - web server services
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
 
#ifndef N_TCP_SERVER_SOCKET_SERVICES
#define N_TCP_SERVER_SOCKET_SERVICES

// Qt
#include <QHttpRequestHeader>
#include <QByteArray>
#include <QString>
#include <QAbstractSocket>
#include <QScriptValue>

// App
#include "n_response.h"
#include "n_services.h"
#include "n_client_session.h"
#include "n_file_category.h"
#include "n_tcp_server_auth_session.h"


#define NTSSERVICES (NTcpServerSocketServices::instance())

class NTcpServerSocketServices: QObject
{
	Q_OBJECT
public:
	static NTcpServerSocketServices & instance();
	static void deleteInstance();
	~NTcpServerSocketServices();
	
	NResponse response(const NClientSession & session);
	void removeExpiredSession();
	
private:	
	static NTcpServerSocketServices *m_instance;
	NTcpServerAuthSessionHash        m_authSessionHash;
	
	NTcpServerSocketServices();
	NResponse & setData(int *statusCode,  NResponse & response,
		const NClientSession & session);
	NResponse & setResponseStatus(int statusCode, NResponse & response);
	
	// Return services help on the same root
	NResponse & svcGetServiceHelp(NService_n::NService* services, NResponse & response);
	// Return sub services help too
	NResponse & svcGetFullServiceHelp(NService_n::NService* rootServices, NResponse & response);
	NResponse & svcGetFavicon(NResponse & response);
	
	// User 
	NResponse & svcUser(const NClientSession & session, NResponse & response);
	NResponse & svcPostUser(const NClientSession & session, NResponse & response);
	NResponse & svcDeleteUser(const NClientSession & session, NResponse & response);
	NResponse & svcGetUser(const NClientSession & session, NResponse & response);
	
	// Authentication
	NResponse & svcAuth(const NClientSession & session, NResponse & response);
	NResponse & svcPostAuth(const NClientSession & session, NResponse & response);
	NResponse & svcDeleteAuth(const NClientSession & session, NResponse & response);
	
	// Nop
	NResponse & svcGetNop(NResponse & response);
	
	// Look for modification
	NResponse & svcLookForModification(NResponse & response);
	
	// file
	NResponse & svcGetSearch(const NClientSession & session, NResponse & response,
							  NFileCategory_n::FileCategory category);
	NResponse & svcGetFileDownload(int *statusCode, const NClientSession & session, NResponse & response);
	
	// Picture
	NResponse & svcGetPictureResize(const NClientSession & session, NResponse & response);
	NResponse & svcGetPictureThumb(const NClientSession & session, NResponse & response);
	
	
	// duplicated files
	NResponse & svcGetDuplicatedFile(const NClientSession & session, NResponse & response,
									  NFileCategory_n::FileCategory category);
		
	// log
	NResponse & svcLog(const NClientSession & session, NResponse & response);
	NResponse & svcDeleteLog(NResponse & response);
	NResponse & svcGetLog(const NClientSession & session, NResponse & response);
	
	// UI
	NResponse & svcRedirectToTUI(int *statusCode, const NClientSession & session,
		NResponse & response);
	NResponse & svcGetUI(int *statusCode,	const NClientSession & session, NResponse & response);
	
	// Shared Dir
	NResponse & svcSharedDir(const NClientSession & session, NResponse & response);
	NResponse & svcPostSharedDir(const NClientSession & session, NResponse & response);
	NResponse & svcPutSharedDir(const NClientSession & session, NResponse & response);
	NResponse & svcGetSharedDir(const NClientSession & session, NResponse & response);
	NResponse & svcDeleteSharedDir(const NClientSession & session, NResponse & response);

	// Music
	NResponse & svcGetMusicAlbum(const NClientSession & session, NResponse & response);
	NResponse & svcGetMusicArtist(const NClientSession & session, NResponse & response);
	NResponse & svcGetMusicGenre(const NClientSession & session, NResponse & response);
	NResponse & svcGetMusicTitle(const NClientSession & session, NResponse & response);
	NResponse & svcGetMusicYear( const NClientSession & session, NResponse & response);
	NResponse & svcGetMusicID3Picture(int *statusCode, const NClientSession & session,
									  NResponse & response);

	// @Return {bool} true, if auth test succeed and httpMethode test succeed
	bool isServiceAvailable(int * statusCode, const NClientSession & session,
							NService_n::NService service);
	// Return, SUCCESS, MSG and totalCount json
	void setJsonRootReponse(QScriptValue & svRoot, int totalCount, bool succeed);
};

#endif //N_TCP_SERVER_SOCKET_SERVICES
