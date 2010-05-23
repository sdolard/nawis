//Qt
#include <QDir>
#include <QCoreApplication>

// Std
#include <iostream>

// App
#include "n_config.h"
#include "n_database.h"
#include "n_log.h"
#include "n_version.h"
#include "n_log_database.h"
#include "n_path.h"

#include "n_server.h"

#define TIMER_INTERVAL     1000

NServer * NServer::m_instance = NULL;

NServer & NServer::instance(QObject *parent)
{
    if (m_instance == NULL)
        m_instance = new NServer(parent);
    return *m_instance;
}

void NServer::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NServer::NServer(QObject *parent)
    :QObject(parent)
{
    m_configFileChanged    = false;
    m_currentJob           = JT_NONE;
    m_server               = NULL;
    m_sslServer            = NULL;
    m_dbUpdaterJob         = NULL;
    m_dirWatcherJob        = NULL;
    m_hasherJob            = NULL;
    m_metadataUpdaterJob   = NULL;
    m_musicDbUpdaterJob    = NULL;
    m_sharedDirectories    = NCONFIG.sharedDirectories();
    m_fileSuffixes         = NCONFIG.fileSuffixes();

    NConfig::instance();

    connect(&NCONFIG, SIGNAL(configFileChanged()),
            this, SLOT(onConfigFileChanged()));
    connect(&m_jobTimer, SIGNAL(timeout()),
            this, SLOT(onJobTimerTimeout()));
}

NServer::~NServer()
{	
    NLOGMD("Server", "Stopping...");
    disconnect();
    m_jobTimer.stop();
    m_configFileChanged = false;
    stopJobs();
    stopTcpServer();
    stopSslTcpServer();
    NDatabase::deleteInstance();
    NConfig::deleteInstance();
    NLOGDB.deleteInstance();
}  

bool NServer::start()
{
    NLOGMD("Server: configuration file path", NPath_n::config());

    // Start DB: must be created as first
    NDatabase::instance();

    bool started = false;
    if (!NCONFIG.isOnlySslServerEnabled())
        started = startTcpServer();
    if (NCONFIG.isSslServerEnabled())
        started = started && startSslTcpServer();

    if (!started)
        return false;

    NCONFIG.dumpSharedDirectoriesConfig();

    m_jobTimer.start(TIMER_INTERVAL);

    return true;
}  

bool NServer::stop()
{
    stopTcpServer();
    stopSslTcpServer();
    return true;
}

bool NServer::pause()
{
    stopTcpServer();
    stopSslTcpServer();
    return true;
}

bool NServer::resume()
{
    return restartTcpServer() && restartSslTcpServer();
}

void NServer::stopTcpServer()
{
    // It's not a thread, this code is ok
    if (m_server == NULL)
        return;
    NLOGMD("Server", tr("TCP server is stopping..."));
    delete m_server;
    m_server = NULL;
    NLOGMD("Server", tr("TCP server is stopped"));
}

void NServer::stopSslTcpServer()
{
    // It's not a thread, this code is ok
    if ( m_sslServer == NULL)
        return;
    NLOGMD("Server", tr("SSL TCP server is stopping..."));
    delete m_sslServer;
    m_sslServer = NULL;
    NLOGMD("Server", tr("SSL TCP server is stopped"));
}

bool NServer::restartTcpServer()
{	
    stopTcpServer();
    if (!NCONFIG.isOnlySslServerEnabled())
        return startTcpServer();
    return false;
}

bool NServer::restartSslTcpServer()
{
    stopSslTcpServer();
    if (NCONFIG.isSslServerEnabled())
        return  startSslTcpServer();
    return false;
}

bool NServer::startTcpServer()
{
    if (m_server) // already running
        return true;

    NLOGMD("Server", tr("TCP server %1 is starting...").arg(NVersion_n::namedVersion(false)));
    m_server = new NTcpServer(NCONFIG.serverPort(), false, this);

    if (m_server->start())
    {
        NLOGMD("Server", tr("TCP Server is listening on %1").arg(m_server->serverPort()));
        NLOGMD("Server", tr("TCP Server is waiting for connection..."));
        return true;
    }

    NLOGMD("Server", tr("TCP server not able to listen on %1: %2\n").
                     arg(NCONFIG.serverPort()).
                     arg(m_server->errorString()));

    NLOGMD("Solution: ", socketErrorToString(m_server->serverError()));
    return false;
}

bool NServer::startSslTcpServer()
{
    if (m_sslServer) // already running
        return true;

    NLOGMD("Server", tr("SSL TCP server %1 is starting...").arg(NVersion_n::namedVersion(false)));
    m_sslServer = new NTcpServer(NCONFIG.serverSslPort(), true, this);

    if (m_sslServer->start())
    {
        NLOGMD("Server", tr("SSL TCP Server is listening on %1").arg(m_sslServer->serverPort()));
        NLOGMD("Server", tr("SSL TCP Server is waiting for connection..."));
        return true;
    }

    NLOGMD("Server", tr("SSL TCP server not able to listen on %1: %2\n").
                     arg(NCONFIG.serverSslPort()).
                     arg(m_sslServer->errorString()));

    NLOGMD("Solution: ", socketErrorToString(m_sslServer->serverError()));
    return false;
}

QString NServer::socketErrorToString(QAbstractSocket::SocketError error)
{
    switch (error)
    {
    case QAbstractSocket::AddressInUseError:
        return  tr("Try to run server on another port");

    case QAbstractSocket::ConnectionRefusedError:
        return tr("Connection Refused Error");

    case QAbstractSocket::SocketAccessError:
        return tr("Set an other server port.");

    case QAbstractSocket::SocketResourceError:
        return tr("Socket Resource Error");

    case QAbstractSocket::SocketTimeoutError:
        return tr("Socket Timeout Error");

    case QAbstractSocket::DatagramTooLargeError:
        return tr("Datagram Too Large Erro");

    case QAbstractSocket::NetworkError:
        return tr("Network Error");

    case QAbstractSocket::SocketAddressNotAvailableError:
        return tr("Socket Address Not Available Error");

    case QAbstractSocket::UnsupportedSocketOperationError:
        return tr("Unsupported Socket Operation Error");

    case QAbstractSocket::UnfinishedSocketOperationError:
        return tr("Unfinished Socket Operation Error");

    case QAbstractSocket::ProxyAuthenticationRequiredError:
        return tr("Proxy Authentication Required Error");

    case QAbstractSocket::UnknownSocketError:
        return tr("Unknown Socket Error");

    case QAbstractSocket::RemoteHostClosedError:
        return tr("Remote Host Closed Error");

    case QAbstractSocket::HostNotFoundError:
        return tr("Host Not Found Error");

    case QAbstractSocket::SslHandshakeFailedError:
        return tr("The SSL/TLS handshake failed.");

#if QT_VERSION >= 0x040500
    case QAbstractSocket::ProxyConnectionRefusedError:
        return tr("Proxy Connection Refused Error");

    case QAbstractSocket::ProxyConnectionClosedError:
        return tr("Proxy Connection Closed Error");

    case QAbstractSocket::ProxyConnectionTimeoutError:
        return tr("Proxy Connection Timeout Error");

    case QAbstractSocket::ProxyNotFoundError:
        return tr("Proxy Not Found Error");

    case QAbstractSocket::ProxyProtocolError:
        return tr("Proxy Protocol Error");
#endif
    default:
        Q_ASSERT_X(false, "NServer::SocketErrorToString",
                   qPrintable(QString("Unmanaged socker error: %1").arg(error)));
        return tr("Unknown QAbstractSocket error");
    }
}

void NServer::onJobTimerTimeout()
{
    if (m_currentJob != JT_NONE)
        return;
    // We check last job execution
    // if last date update is valid AND delais is not overloaded and config has not changed
    // then we have nothing to do.
    if (NCONFIG.isLastDirUpdateValid())
        return;

    m_jobTimer.stop();
    startJobs();
}

// This is called in timer. timer has already done test.
void NServer::startJobs()
{	
    if (m_configFileChanged)
        return;

    m_currentJob++;
    if (m_currentJob > JT_LAST )
    {
        stopJobs();
        NCONFIG.setLastDirUpdateDone();
        m_jobTimer.start(TIMER_INTERVAL);
        return;
    }
    startJob(m_currentJob);
}

void NServer::startJob(int job)
{	
    stopJob(job);
    m_currentJob = job;
    NThread **pJob = NULL;

    switch (job) {
    case JT_NONE:
        return;

    case JT_WATCH_FILES:
        m_dirWatcherJob = new NDirWatcherThread();
        pJob = (NThread**)&m_dirWatcherJob;
        connect(m_dirWatcherJob, SIGNAL(hash(QString, NDirWatcherThreadItems)),
                this, SLOT(onDirWatcherHash(QString, NDirWatcherThreadItems)), Qt::DirectConnection);
        break;

    case JT_DB_UPDATE:
        m_dbUpdaterJob = new NDbUpdaterThread(m_dirs);
        pJob = (NThread**)&m_dbUpdaterJob;
        break;

    case JT_HASH:
        m_hasherJob = new NHasherThread();
        pJob = (NThread**)&m_hasherJob;
        break;

    case JT_GET_METADATA:
        m_metadataUpdaterJob = new NMetadataUpdaterThread();
        pJob = (NThread**)&m_metadataUpdaterJob;
        break;

    case JT_BUILD_MUSIC_DB:
        m_musicDbUpdaterJob = new NMusicDbUpdaterThread();
        pJob = (NThread**)&m_musicDbUpdaterJob;
        break;

    default:
        Q_ASSERT_X(false, "NServer::stopJob", "job not managed");
        break;
    }

    connect((*pJob), SIGNAL(finished()),
            this, SLOT(onJobTerminated()), Qt::QueuedConnection);
    NLOGD("NServer", QString("%1 starting ...").arg(jobToString(job)));
    (*pJob)->start(QThread::IdlePriority);
}

void NServer::stopJobs()
{          	
    for(int job = 0; job <= JT_LAST; job++)
        stopJob(job);
    m_currentJob = JT_NONE;
}

void NServer::stopJob(int job)
{
    NThread **pJob = NULL;

    switch (job) {
    case JT_NONE:
        return;

    case JT_WATCH_FILES:
        pJob = (NThread**)&m_dirWatcherJob;
        break;

    case JT_DB_UPDATE:
        pJob = (NThread**)&m_dbUpdaterJob;
        break;

    case JT_HASH:
        pJob = (NThread**)&m_hasherJob;
        break;

    case JT_GET_METADATA:
        pJob = (NThread**)&m_metadataUpdaterJob;
        break;

    case JT_BUILD_MUSIC_DB:
        pJob = (NThread**)&m_musicDbUpdaterJob;
        break;

    default:
        Q_ASSERT_X(false, "NServer::stopJob", "job not managed");
        break;
    }

    if (*pJob == NULL)
        return;

    NLOGD("NServer", QString("%1 stopping ...").arg(jobToString(job)));
    if ((*pJob)->isRunning())
    {
        (*pJob)->stop();
        while (!(*pJob)->isFinished())
            (*pJob)->wait(100);
    }
    (*pJob)->disconnect();
    delete (*pJob);
    (*pJob) = NULL;
    NLOGD("NServer", QString("%1 stopped").arg(jobToString(job)));
}

void NServer::onJobTerminated()
{
    stopJob(m_currentJob);
    startJobs();
}

void NServer::onDirWatcherHash(QString hash, NDirWatcherThreadItems dirs)
{
    Q_ASSERT(m_currentJob == JT_WATCH_FILES);

    if (NCONFIG.dirFingerPrint() == hash &&
        NCONFIG.isLastDirUpdateValid())
    {
        // No update needed
        m_currentJob = JT_LAST;
        return;
    }

    NCONFIG.setDirFingerPrint(hash);
    m_dirs = dirs;
}

void NServer::onConfigFileChanged()
{
    if ((m_server && NCONFIG.isOnlySslServerEnabled()) ||
          (!m_server && !NCONFIG.isOnlySslServerEnabled()) ||
          (m_server && NCONFIG.serverPort() != m_server->serverPort())
        )
        restartTcpServer();

    if ((m_sslServer && !NCONFIG.isSslServerEnabled()) ||
         (!m_sslServer && NCONFIG.isSslServerEnabled()) ||
         (m_sslServer && NCONFIG.serverSslPort() != m_sslServer->serverPort())
        )
        restartSslTcpServer();

    bool sharedDirectoriesChanged = NCONFIG.sharedDirectories() != m_sharedDirectories;

    if (sharedDirectoriesChanged)
        NCONFIG.dumpSharedDirectoriesConfig();

    bool fileSuffixesChanged = NCONFIG.fileSuffixes() != m_fileSuffixes;
    if (!sharedDirectoriesChanged &&
        !fileSuffixesChanged)
        return;

    NLOGM("Server", "Configuration changed");
    m_configFileChanged = true;
    m_jobTimer.stop();
    stopJobs();
    NCONFIG.invalidLastDirUpdate();
    if (sharedDirectoriesChanged)
    {
        m_sharedDirectories = NCONFIG.sharedDirectories();
        NCONFIG.dumpSharedDirectoriesConfig();
    }

    if (fileSuffixesChanged)
        m_fileSuffixes = NCONFIG.fileSuffixes();

    m_configFileChanged = false;
    m_jobTimer.start(TIMER_INTERVAL);
}

int NServer::jobStatus() const
{
    return m_currentJob;
}

const QString NServer::jobToString(int job)
{
    switch (job) {
    case JT_NONE: return "JT_NONE";
    case JT_WATCH_FILES: return "JT_WATCH_FILES";
    case JT_DB_UPDATE: return "JT_DB_UPDATE";
    case JT_HASH: return "JT_HASH";
    case JT_GET_METADATA: return "JT_GET_METADATA";
    case JT_BUILD_MUSIC_DB: return "JT_BUILD_MUSIC_DB";
    default:
        Q_ASSERT_X(false, "NServer::jobToString", "Conversion missing.");
        return "";
    }
}
