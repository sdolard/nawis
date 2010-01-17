#ifndef N_TCP_SERVER_SOCKET_LIST_H
#define N_TCP_SERVER_SOCKET_LIST_H

// Qt
#include <QList>

// App
#include "n_tcp_server_socket.h"

class NTcpServerSocketList: public QList<NTcpServerSocket*>{};

#endif // N_TCP_SERVER_SOCKET_LIST_H
