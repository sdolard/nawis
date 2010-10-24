#ifndef N_TCP_SERVER_SOCKET_CFG_SERVICES_H
#define N_TCP_SERVER_SOCKET_CFG_SERVICES_H

#include "n_tcp_server_socket_service.h"

class NTcpServerSocketCfgServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketCfgServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketCfgServices();

    // Shared Dir
    NResponse & sharedDir(const NClientSession & session, NResponse & response);

    NResponse & lookForModification(NResponse & response);

    NResponse & getAvailableLevelList(NResponse & response);

private:
    static NTcpServerSocketCfgServices *m_instance;
    NTcpServerSocketCfgServices();

    NResponse & postSharedDir(const NClientSession & session, NResponse & response);
    NResponse & putSharedDir(const NClientSession & session, NResponse & response);
    NResponse & getSharedDir(const NClientSession & session, NResponse & response);
    NResponse & deleteSharedDir(const NClientSession & session, NResponse & response);

};

inline NTcpServerSocketCfgServices & getCfgServices(){
    return NTcpServerSocketCfgServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_CFG_SERVICES_H
