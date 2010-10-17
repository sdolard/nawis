// Qt
#include <QPair>
#include <QHostAddress>
#include <QHttpResponseHeader>
#include <QDateTime>

// App
#include "n_log.h"
#include "n_http.h"
#include "n_version.h"
#include "n_tcp_server_socket_services.h"
#include "n_client_session.h"
#include "n_config.h"

#include "n_tcp_server_socket.h"

NTcpServerSocket::NTcpServerSocket(bool ssl, QObject * parent)
    :QObject(parent)
{
    m_ssl = ssl;
    m_keepAliveTimer.setInterval ( 30000 ); // 30s
    m_dataToRead = 0;
    m_state = NTcpServerSocket::ssReadingLine;
}
NTcpServerSocket::~NTcpServerSocket()
{
    // Socket is self deleted througth deleleLater slot called by disconnected socket signal
}

bool NTcpServerSocket::setSocketDescriptor(int socketDescriptor)
{
    m_keepAliveTimer.start();

    if (!connectSignals())
        return false;

    if (m_ssl) {
        if(!initSsl())
            return false;

        if (!connectSslSignals())
            return false;
    }
    bool ok = m_sslSocket.setSocketDescriptor(socketDescriptor);

    if (m_ssl)
        m_sslSocket.startServerEncryption();

    return ok;
}

bool NTcpServerSocket::connectSslSignals()
{
    // Ssl event
    return connect(&m_sslSocket, SIGNAL(encryptedBytesWritten ( qint64 )),
                   this, SLOT(sslEncryptedBytesWritten ( qint64 )), Qt::QueuedConnection) &&
    connect(&m_sslSocket, SIGNAL(modeChanged ( QSslSocket::SslMode )), // Not used
            this, SLOT(sslModeChanged ( QSslSocket::SslMode )), Qt::QueuedConnection) &&
    connect(&m_sslSocket, SIGNAL(peerVerifyError ( const QSslError )),
            this, SLOT(sslPeerVerifyError ( const QSslError & )), Qt::QueuedConnection) &&
    connect(&m_sslSocket, SIGNAL(sslErrors ( const QList<QSslError> & )),
            this, SLOT(sslErrors ( const QList<QSslError>  &)), Qt::QueuedConnection);
}

bool NTcpServerSocket::connectSignals()
{
    return connect(&m_sslSocket, SIGNAL(readyRead()), // readyRead
                   this, SLOT(socketReadyRead()), Qt::QueuedConnection) &&
    connect(&m_sslSocket, SIGNAL(bytesWritten ( qint64 )), // bytesWritten
            this, SLOT(bytesWritten ( qint64 )), Qt::QueuedConnection) &&
    connect(&m_sslSocket, SIGNAL(disconnected()), // disconnected
            this, SLOT(deleteLater()), Qt::QueuedConnection) &&
    connect(&m_keepAliveTimer, SIGNAL(timeout()), // timeout
            &m_sslSocket, SLOT(disconnectFromHostImplementation()), Qt::QueuedConnection);
}

void NTcpServerSocket::read()
{
    if (m_state !=  NTcpServerSocket::ssReadingLine &&
        m_state !=  NTcpServerSocket::ssReadingData)
        return;

    if (!isConnected())
        return;

    if (!m_sslSocket.bytesAvailable())
        return;

    while (m_sslSocket.bytesAvailable())
    {
        QByteArray data;
        switch (m_state) {
        case NTcpServerSocket::ssReadingLine:
            if (!m_sslSocket.canReadLine())
                return;
            addLine(m_sslSocket.readLine());
            break;
        case NTcpServerSocket::ssReadingData:
            data = m_sslSocket.read(m_dataToRead);
            m_dataToRead = m_dataToRead - data.length();
            addContent(data);
            break;
        default:
            break;
        }
        if (m_state == NTcpServerSocket::ssWriting)
            break;
    }

    if (!requestIsComplete())
        return;

    prepareResponse();
}

void NTcpServerSocket::write()
{
    if (m_state !=  NTcpServerSocket::ssWriting)
        return;

    if (!isConnected())
        return;

    sendResponse();
}

const QString NTcpServerSocket::peerString()
{
    QString peer;
    peer = m_sslSocket.peerName();
    if (!peer.isEmpty())
        return peer;
    return m_sslSocket.peerAddress().toString();

}

void NTcpServerSocket::addLine(const QString & line)
{
    Q_ASSERT(m_state == NTcpServerSocket::ssReadingLine);
    /*
    logDebug("NTcpServerSocket", QString("(%1) addLine: %2").arg(m_sslSocket.socketDescriptor()).
                   arg(QUrl::fromPercentEncoding(line.toAscii())));
                   */

    m_response.data().append(line);
    if (line == "\r\n")
    {
        m_request = QHttpRequestHeader(m_response.data());
        m_keepAlive = m_request.value("Connection").compare("keep-alive", Qt::CaseInsensitive) == 0;
        if (m_keepAlive)
        {
            if (m_request.hasKey("Keep-Alive"))
            {
                bool ok;
                int clientKeepAliveTimeout = m_request.value("Keep-Alive").toInt(&ok);
                if (ok && (clientKeepAliveTimeout * 100) < m_keepAliveTimer.interval())
                    m_keepAliveTimer.setInterval(clientKeepAliveTimeout * 100);
            }
        }

        if (m_request.method() == "GET" || m_request.method() == "DELETE")
        {
            m_response.data().clear();
            m_state = NTcpServerSocket::ssWriting;
        } else if (m_request.method() == "POST" || m_request.method() == "PUT")
        {
            m_dataToRead = m_request.contentLength();
            m_state = NTcpServerSocket::ssReadingData;
        } else {
            m_content.clear();
            m_request = QHttpRequestHeader();
            disconnect();
        }
    }
    m_keepAliveTimer.start();
}


void NTcpServerSocket::addContent(const QByteArray & data)
{
    Q_ASSERT(m_state == NTcpServerSocket::ssReadingData);

    m_content += data;
    if (m_dataToRead == 0) {
        m_response.data().clear();
        m_content = QByteArray::fromPercentEncoding(m_content);
        m_state = NTcpServerSocket::ssWriting;
    }

    m_keepAliveTimer.start();
}

bool NTcpServerSocket::requestIsComplete()
{
    return m_state == NTcpServerSocket::ssWriting;
}

void NTcpServerSocket::prepareResponse()
{
    Q_ASSERT(m_state == NTcpServerSocket::ssWriting);
    // To thread this?
    m_response =  NTcpServerSocketServices::instance().getResponse(NClientSession(m_request, m_content, &m_sslSocket, m_ssl));
    m_content.clear();
    m_request = QHttpRequestHeader();
}

void NTcpServerSocket::sendResponse()
{
    Q_ASSERT(m_state == NTcpServerSocket::ssWriting);

    qint64 written = 0;
    if ((m_sslSocket.bytesToWrite() == 0) &&
        (m_sslSocket.encryptedBytesToWrite() == 0))
    {
        // we write if socket buffer is empty
        bool error = false;
        if (m_response.dataType() == NResponse::dtFile)
            error = m_response.updateFileBuffer() == -1; // Buffer update

        if(!error)
            written = m_sslSocket.write(m_response.data().constData(), m_response.data().size());
    }

    if (written == 0)
        return;

    m_response.data().remove(0, written);
    if (m_response.data().length() == 0)
    {
        switch (m_response.dataType()) {
        case NResponse::dtPriorBuffered:
            m_state = NTcpServerSocket::ssReadingLine;
            if (!m_keepAlive | !m_response.keepAlive())
                disconnectFromHost();
            break;
        case NResponse::dtFile:
            if (m_response.updateFileBuffer() <= 0)
            {
                m_state = NTcpServerSocket::ssReadingLine;
                if (!m_keepAlive | !m_response.keepAlive())
                    disconnectFromHost();
            }
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
}

bool NTcpServerSocket::isConnected()
{
    return m_sslSocket.state() == QAbstractSocket::ConnectedState;
}

bool NTcpServerSocket::isUnconnected()
{
    return m_sslSocket.state() == QAbstractSocket::UnconnectedState;
}

void NTcpServerSocket::disconnectFromHost()
{
    m_sslSocket.disconnectFromHost();
}

void NTcpServerSocket::socketReadyRead()
{
    read();
    write();
}

void NTcpServerSocket::bytesWritten ( qint64 )
{
    m_keepAliveTimer.start();
    write();
}

void NTcpServerSocket::sslEncryptedBytesWritten ( qint64 )
{
    m_keepAliveTimer.start();
    write();
}

void NTcpServerSocket::sslEncryptiondReady()
{
    logDebug("NTcpServerSocket", "sslEncryptiondReady");
}

void NTcpServerSocket::sslModeChanged ( QSslSocket::SslMode mode )
{
    switch (mode) {
    case QSslSocket::UnencryptedMode:
        logDebug("sslModeChanged", "The socket is unencrypted. Its behavior is identical to QTcpSocket.");
        break;
    case QSslSocket::SslClientMode:
        logDebug("sslModeChanged", "The socket is a client-side SSL socket. It is either alreayd encrypted, or it is in the SSL handshake phase (see QSslSocket::isEncrypted())");
        break;
    case QSslSocket::SslServerMode:
        logDebug("sslModeChanged", "The socket is a server-side SSL socket. It is either already encrypted, or it is in the SSL handshake phase (see QSslSocket::isEncrypted()).");
        break;
    default:
        Q_ASSERT_X(false, "NTcpServerSocket","sslModeChanged: missing ssl mode");
        break;
    }
}

void NTcpServerSocket::sslPeerVerifyError ( const QSslError &  error)
{
    logDebug("sslPeerVerifyError", error.errorString());
}

void NTcpServerSocket::sslErrors ( const QList<QSslError> & errors )
{
    QStringList sslErrors;
    foreach(QSslError error, errors){
        sslErrors.append(error.errorString());
    }
    logDebug("sslPeerVerifyError", sslErrors.join("\n"));
}

bool NTcpServerSocket::initSsl()
{
    m_sslSocket.setSslConfiguration(getConfig().sslCfg());
    return !getConfig().sslCfg().isNull();
}
