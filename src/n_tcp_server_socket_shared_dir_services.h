#ifndef N_TCP_SERVER_SOCKET_SHARED_DIR_SERVICES_H
#define N_TCP_SERVER_SOCKET_SHARED_DIR_SERVICES_H

#include "n_tcp_server_socket_service.h"

class NTcpServerSocketSharedDirServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketSharedDirServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketSharedDirServices();

    // Shared Dir
    NResponse & sharedDir(const NClientSession & session, NResponse & response);

private:
    static NTcpServerSocketSharedDirServices *m_instance;
    NTcpServerSocketSharedDirServices();

    NResponse & postSharedDir(const NClientSession & session, NResponse & response);
    NResponse & putSharedDir(const NClientSession & session, NResponse & response);
    NResponse & getSharedDir(const NClientSession & session, NResponse & response);
    NResponse & deleteSharedDir(const NClientSession & session, NResponse & response);
};

inline NTcpServerSocketSharedDirServices & getSharedDirServices(){
    return NTcpServerSocketSharedDirServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_SHARED_DIR_SERVICES_H
