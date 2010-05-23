// Std
#include <iostream>

// Qt
#include <QTcpSocket>
#include <QEventLoop>
#include <QCoreApplication>

// App
#include "n_config.h"
#include "n_log.h"
#include "n_tcp_server_socket.h"
#include "n_tcp_server_socket_services.h"

#include "n_tcp_server.h"

/*******************************************************************************
*  NTcpServer
*******************************************************************************/
NTcpServer::NTcpServer(quint16 port, bool ssl, QObject *parent)
    :QTcpServer(parent),
    m_port(port),
    m_ssl(ssl)

{
    m_tcpServerThread = new NTcpServerThread(this, this, m_ssl);
    m_tcpServerThread->start();
}

NTcpServer::~NTcpServer()
{
    if (m_tcpServerThread->isRunning())
    {
        m_tcpServerThread->quit();
        while (!m_tcpServerThread->isFinished())
            m_tcpServerThread->wait(100);
        delete m_tcpServerThread;
    }
}

bool NTcpServer::start()
{
    return listen(QHostAddress::Any, m_port);
}

void NTcpServer::incomingConnection(int socketDescriptor)
{
    emit newIncommingConnection(socketDescriptor);
}


/*******************************************************************************
*  NTcpServerThread
*******************************************************************************/
NTcpServerThread::NTcpServerThread(QObject * parent, NTcpServer *tcpServer, bool ssl)
    :NThread(parent), m_tcpServer(tcpServer), m_ssl(ssl)
{
    Q_ASSERT(tcpServer != NULL);
}

void NTcpServerThread::run()
{
    NSocketManager socketManager(m_ssl);
    connect(m_tcpServer, SIGNAL(newIncommingConnection(int)),
            &socketManager, SLOT(newConnection(int)), Qt::QueuedConnection);

    // Auth expiration
    m_authExpirationTimer = new QTimer();
    m_authExpirationTimer->setInterval(30000); // 30s
    connect(m_authExpirationTimer, SIGNAL(timeout()),
            &socketManager, SLOT(onTimeToChechAuth()), Qt::QueuedConnection);
    m_authExpirationTimer->start();
    exec();
}

/*******************************************************************************
*  NSocketManager
*******************************************************************************/
void NSocketManager::newConnection(int socketDescriptor)
{
    NTcpServerSocket *socket = new NTcpServerSocket(m_ssl);
    if (!socket->setSocketDescriptor(socketDescriptor))
    {
        NLOGM("NTcpServerThread",
              QString("bindSocketDescriptors() setSocketDescriptor failed (ssl: %1)").
              arg(QVariant(m_ssl).toString()));
        delete socket;
    }
}

void NSocketManager::onTimeToChechAuth()
{
    NTSSERVICES.removeExpiredSession();
}
