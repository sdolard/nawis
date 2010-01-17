#ifndef N_TCP_SERVER_H
#define N_TCP_SERVER_H

// Qt
#include <QPair>
#include <QTcpServer>
#include <QTimer>

#include "n_thread.h"

class NTcpServerThread;

class NTcpServer : public QTcpServer
{
	Q_OBJECT
public:
	NTcpServer(QObject *parent = 0);
	~NTcpServer();
	
	bool start();

signals:
	void newIncommingConnection(int socketDescriptor);
	
private:
	NTcpServerThread *m_tcpServerThread;
	
protected:
    void incomingConnection(int socketDescriptor);
};

class NSocketManager: public QObject {
	Q_OBJECT

public:
	NSocketManager(bool ssl):m_ssl(ssl){};

public slots:
	void newConnection(int socketDescriptor);
	void onTimeToChechAuth();

private:
	bool m_ssl;
};

class NTcpServerThread: public NThread
{
	Q_OBJECT
public:
	NTcpServerThread(QObject * parent, NTcpServer *tcpServer);

protected:
	void run();

private:
	bool             m_ssl;
	NTcpServer     *m_tcpServer;
	QTimer          *m_authExpirationTimer;
};


#endif // N_TCP_SERVER_H
