#ifndef N_TCP_SERVER_SOCKET_H
#define N_TCP_SERVER_SOCKET_H

// Qt
#include <QSslSocket>
#include <QStringList>
#include <QHttpRequestHeader>
#include <QTimer>

// App
#include "n_response.h"

// TODO: QoS on write sockets in order to manager cpu overload
class NTcpServerSocket: public QObject
{
    Q_OBJECT
public:
    enum State {
        ssReadingData,
        ssReadingLine,
        ssWriting
    };
    NTcpServerSocket(bool ssl, QObject * parent = 0 );
    ~NTcpServerSocket();

    bool setSocketDescriptor(int socketDescriptor);

    void read();
    void write();
    bool isUnconnected();
    bool isConnected();
    void disconnectFromHost();

private slots:
    void socketReadyRead();
    void bytesWritten ( qint64 bytes );
    // Ssl
    void sslEncryptedBytesWritten ( qint64 bytes );
    void sslEncryptiondReady();
    void sslModeChanged ( QSslSocket::SslMode mode );
    void sslPeerVerifyError ( const QSslError & error );
    void sslErrors ( const QList<QSslError> & errors );

private:
    bool                     m_ssl;
    QSslSocket               m_sslSocket;
    QTimer                   m_keepAliveTimer;
    NResponse                m_response;
    QHttpRequestHeader       m_request;
    NTcpServerSocket::State  m_state;
    bool                     m_keepAlive;
    bool                     m_canBeDeleted;
    uint                     m_dataToRead;
    QByteArray               m_postData;

    // Return hostName if exists, ip address otherwise
    const QString peerString();

    void addLine(const QString & line);
    void addData(const QByteArray & data);
    bool requestIsComplete();
    void sendResponse();
    void prepareResponse();
    bool initSsl();
    bool connectSslSignals();
    bool connectSignals();
};

#endif //N_TCP_SERVER_SOCKET_H

