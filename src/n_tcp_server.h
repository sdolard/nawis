#ifndef N_TCP_SERVER_H
#define N_TCP_SERVER_H

// Qt
#include <QPair>
#include <QTcpServer>
#include <QTimer>

#include "n_thread.h"


/**
* NTcpServer
*   NTcpServerThread
*       NSocketManager
*/

class NTcpServerThread;

/**
* NTcpServer
*/
class NTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    NTcpServer(quint16 port, bool ssl, QObject *parent = 0);
    ~NTcpServer();

    bool start();

signals:
    void newIncommingConnection(int socketDescriptor);

private:
    quint16           m_port;
    bool              m_ssl;
    NTcpServerThread *m_tcpServerThread;

protected:
    void incomingConnection(int socketDescriptor);
};

/**
* NTcpServerThread
*/
class NTcpServerThread: public NThread
{
    Q_OBJECT
public:
    NTcpServerThread(QObject * parent, NTcpServer *tcpServer, bool ssl);

protected:
    void run();

private:
    NTcpServer     *m_tcpServer;
    bool            m_ssl;
    QTimer         *m_authExpirationTimer;
};


/**
* NSocketManager
*/
class NSocketManager: public QObject {
    Q_OBJECT

public:
    NSocketManager(bool ssl):m_ssl(ssl){}

public slots:
    void newConnection(int socketDescriptor);
    void onTimeToChechAuth();

private:
    bool m_ssl;
};


#endif // N_TCP_SERVER_H
