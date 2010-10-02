#ifndef N_TCP_SERVER_SOCKET_UI_SERVICES_H
#define N_TCP_SERVER_SOCKET_UI_SERVICES_H


#include "n_tcp_server_socket_service.h"

class NTcpServerSocketUIServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketUIServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketUIServices();

    NResponse & getFavicon(NResponse & response);
    // Nop
    NResponse & nop(NResponse & response);
    // UI
    NResponse & redirectToTUI(int *statusCode, const NClientSession & session,
                                 NResponse & response);
    NResponse & getUI(int *statusCode,	const NClientSession & session, NResponse & response);

    NResponse & lookForModification(NResponse & response);

private:
    static NTcpServerSocketUIServices *m_instance;
    NTcpServerSocketUIServices();

};

inline NTcpServerSocketUIServices & getUIServices(){
    return NTcpServerSocketUIServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_UI_SERVICES_H
