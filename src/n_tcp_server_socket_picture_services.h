#ifndef N_TCP_SERVER_SOCKET_PICTURE_SERVICES_H
#define N_TCP_SERVER_SOCKET_PICTURE_SERVICES_H

#include "n_tcp_server_socket_service.h"

class NTcpServerSocketPictureServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketPictureServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketPictureServices();

    // Picture
    NResponse & resize(const NClientSession & session, NResponse & response);
    NResponse & getThumb(const NClientSession & session, NResponse & response);

private:
    static NTcpServerSocketPictureServices *m_instance;
    NTcpServerSocketPictureServices();

};

inline NTcpServerSocketPictureServices & getPictureServices(){
    return NTcpServerSocketPictureServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_PICTURE_SERVICES_H
