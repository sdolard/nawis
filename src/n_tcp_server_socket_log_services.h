#ifndef N_TCP_SERVER_SOCKET_LOG_SERVICES_H
#define N_TCP_SERVER_SOCKET_LOG_SERVICES_H

#include "n_tcp_server_socket_service.h"

class NTcpServerSocketLogServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketLogServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketLogServices();

    // log
    NResponse & log(const NClientSession & session, NResponse & response);

private:
    static NTcpServerSocketLogServices *m_instance;
    NTcpServerSocketLogServices();

    NResponse & deleteLog(NResponse & response);
    NResponse & getLog(const NClientSession & session, NResponse & response);
};

inline NTcpServerSocketLogServices & getLogServices(){
    return NTcpServerSocketLogServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_LOG_SERVICES_H
