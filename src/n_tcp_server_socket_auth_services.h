#ifndef N_TCP_SERVER_SOCKET_AUTH_SERVICES_H
#define N_TCP_SERVER_SOCKET_AUTH_SERVICES_H

#include "n_tcp_server_socket_service.h"
#include "n_tcp_server_auth_session.h"

class NTcpServerSocketAuthServices: public NTcpServerSocketService {
public:
    static NTcpServerSocketAuthServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketAuthServices();

    NResponse & auth(const NClientSession & session, NResponse & response);

    const NTcpServerAuthSession getSession(const QString & sessionId) const;

    bool isSessionValid(const NClientSession & session, int requiredLevel = AUTH_LEVEL_NONE);

    bool isSessionValid(const QString & sessionId, const QString address,
                 const QString & userAgent, int requiredLevel = AUTH_LEVEL_NONE);

    void removeExpiredSession();

private:
    static NTcpServerSocketAuthServices *m_instance;

    NTcpServerAuthSessionHash            m_authSessionHash;

    NTcpServerSocketAuthServices();

    NResponse & postAuth(const NClientSession & session, NResponse & response);
    NResponse & deleteAuth(const NClientSession & session, NResponse & response);
};

inline NTcpServerSocketAuthServices & getAuthServices(){
    return NTcpServerSocketAuthServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_AUTH_SERVICES_H
