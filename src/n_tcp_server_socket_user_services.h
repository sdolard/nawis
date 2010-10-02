#ifndef N_TCP_SERVER_SOCKET_USER_SERVICES_H
#define N_TCP_SERVER_SOCKET_USER_SERVICES_H


#include "n_tcp_server_socket_service.h"

class NTcpServerSocketUserServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketUserServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketUserServices();

    // User
    NResponse & user(const NClientSession & session, NResponse & response);

private:
    static NTcpServerSocketUserServices *m_instance;
    NTcpServerSocketUserServices();

    NResponse & postUser(const NClientSession & session, NResponse & response);
    NResponse & putUser(const NClientSession & session, NResponse & response);
    NResponse & deleteUser(const NClientSession & session, NResponse & response);
    NResponse & getUser(const NClientSession & session, NResponse & response);
};

inline NTcpServerSocketUserServices & getUserServices(){
    return NTcpServerSocketUserServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_USER_SERVICES_H
