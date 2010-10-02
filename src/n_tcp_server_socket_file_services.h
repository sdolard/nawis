#ifndef N_TCP_SERVER_SOCKET_FILE_SERVICES_H
#define N_TCP_SERVER_SOCKET_FILE_SERVICES_H

#include "n_file_category.h"

#include "n_tcp_server_socket_service.h"

class NTcpServerSocketFileServices : public NTcpServerSocketService
{
public:
    static NTcpServerSocketFileServices & instance();
    static void deleteInstance();

    ~NTcpServerSocketFileServices();
    // file
    NResponse & search(const NClientSession & session, NResponse & response,
                       NFileCategory_n::FileCategory category);
    NResponse & download(int *statusCode, const NClientSession & session, NResponse & response);

    // duplicated files
    NResponse & getDuplicated(const NClientSession & session, NResponse & response,
                              NFileCategory_n::FileCategory category);

private:
    static NTcpServerSocketFileServices *m_instance;
    NTcpServerSocketFileServices();

};

inline NTcpServerSocketFileServices & getFileServices(){
    return NTcpServerSocketFileServices::instance();
}

#endif // N_TCP_SERVER_SOCKET_FILE_SERVICES_H
