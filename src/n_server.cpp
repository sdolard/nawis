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
	m_configFileChanged = false;
	m_currentJob = JT_NONE;
	m_server = NULL;
	m_dbUpdaterJob = NULL;
	m_dirWatcherJob = NULL;  
	m_hasherJob = NULL;
	m_metadataUpdaterJob = NULL;
	m_sharedDirectories = NCONFIG.sharedDirectories();
	m_fileSuffixes = NCONFIG.fileSuffixes();
	
	NConfig::instance();
	
	connect(&NCONFIG, SIGNAL(configFileChanged()),
			this, SLOT(onConfigFileChanged()));
	connect(&m_jobTimer, SIGNAL(timeout()), 
			this, SLOT(onJobTimerTimeout()));
}

NServer::~NServer()
{	
	NLOGM("NServer", "Stopping...");
	disconnect();
	m_jobTimer.stop();
	m_configFileChanged = false;
	stopJobs();
	stopTcpServer();
	NDatabase::deleteInstance();
	NConfig::deleteInstance();
	NLOGDB.deleteInstance();
	QCoreApplication::quit();
}  

bool NServer::start()
{
	// Start DB: must be created as first
	NDatabase::instance();
	
	if (!startTcpServer())
		return false;
	
	NCONFIG.dumpSharedDirectoriesConfig();
	
	m_jobTimer.start(TIMER_INTERVAL);
	
	return true;
}  

void NServer::stopTcpServer()
{
	// It's not a thread, this code is ok
	if (m_server == NULL)
		return;
	delete m_server; 
	m_server = NULL; 	
}

bool NServer::restartTcpServer()
{	
	NLOGM("NServer", tr("Server %1 is restarting...").arg(NVersion_n::namedVersion(false)));
	stopTcpServer();
	return startTcpServer();
}

bool NServer::startTcpServer()
{
	if (m_server) // already running
		return true;
	
	m_server = new NTcpServer(this);
	m_errorMessage.clear();
	
	if (m_server->start())
	{
		NLOGM("NServer", tr("Server %1 is listening on tcp/%2; ssl: %3").arg(NVersion_n::namedVersion(false)).
			   arg(m_server->serverPort()).arg(QVariant(NCONFIG.isSslServer()).toString()));
		NLOGM("NServer", tr("Waiting for connection..."));
		return true;
	}
	
	m_errorMessage = tr("Server not able to listen on tcp/%1: %2\n").
	arg(NCONFIG.serverPort()).
	arg(m_server->errorString());
	
	m_errorMessage += tr("Solution: ");
	switch (m_server->serverError())
	{
		case QAbstractSocket::AddressInUseError:
			m_errorMessage += tr("Try to run server on another port");
			break;
		case QAbstractSocket::ConnectionRefusedError:
			m_errorMessage += tr("Connection Refused Error");
			break;
		case QAbstractSocket::SocketAccessError:
			m_errorMessage += tr("Set an other server port.");
			break;
		case QAbstractSocket::SocketResourceError:
			m_errorMessage += tr("Socket Resource Error");
			break;
		case QAbstractSocket::SocketTimeoutError:
			m_errorMessage += tr("Socket Timeout Error");
			break;
		case QAbstractSocket::DatagramTooLargeError:
			m_errorMessage += tr("Datagram Too Large Erro");
			break;
		case QAbstractSocket::NetworkError:
			m_errorMessage += tr("Network Error");
			break;
		case QAbstractSocket::SocketAddressNotAvailableError:
			m_errorMessage += tr("Socket Address Not Available Error");
			break;
		case QAbstractSocket::UnsupportedSocketOperationError:
			m_errorMessage += tr("Unsupported Socket Operation Error");
			break;
		case QAbstractSocket::UnfinishedSocketOperationError:
			m_errorMessage += tr("Unfinished Socket Operation Error");
			break;
		case QAbstractSocket::ProxyAuthenticationRequiredError:
			m_errorMessage += tr("Proxy Authentication Required Error");
			break;
		case QAbstractSocket::UnknownSocketError:
			m_errorMessage += tr("Unknown Socket Error");
			break;
		case QAbstractSocket::RemoteHostClosedError:
			m_errorMessage += tr("Remote Host Closed Error");
			break;
		case QAbstractSocket::HostNotFoundError:
			m_errorMessage += tr("Host Not Found Error");
			break;
		case QAbstractSocket::SslHandshakeFailedError:
			m_errorMessage += tr("The SSL/TLS handshake failed.");
			break;
#if QT_VERSION >= 0x040500
		case QAbstractSocket::ProxyConnectionRefusedError:
			m_errorMessage += tr("Proxy Connection Refused Error");
			break;
		case QAbstractSocket::ProxyConnectionClosedError:
			m_errorMessage += tr("Proxy Connection Closed Error");
			break;
		case QAbstractSocket::ProxyConnectionTimeoutError:
			m_errorMessage += tr("Proxy Connection Timeout Error");
			break;
		case QAbstractSocket::ProxyNotFoundError:
			m_errorMessage += tr("Proxy Not Found Error");
			break;
		case QAbstractSocket::ProxyProtocolError:
			m_errorMessage += tr("Proxy Protocol Error");
			break;
#endif
	}
	m_errorMessage += "\n";
	return false;
}             

QString & NServer::errorMessage()
{          
	return m_errorMessage;
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
	if (NCONFIG.serverPort() != m_server->serverPort())
	{
		restartTcpServer();
		NCONFIG.dumpSharedDirectoriesConfig();
	}
	
	bool sharedDirectoriesChanged = NCONFIG.sharedDirectories() != m_sharedDirectories;
	bool fileSuffixesChanged = NCONFIG.fileSuffixes() != m_fileSuffixes;
	if (!sharedDirectoriesChanged && 
		!fileSuffixesChanged)
		return;
	
	NLOGM("NServer", "Configuration changed");
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
		case JT_NONE:
			return "JT_NONE"; 
			break;
		case JT_WATCH_FILES:
			return "JT_WATCH_FILES"; 
			break;
		case JT_DB_UPDATE:
			return "JT_DB_UPDATE"; 
			break;
		case JT_HASH:
			return "JT_HASH"; 
			break;
		case JT_GET_METADATA:
			return "JT_GET_METADATA"; 
			break;
		default:
			Q_ASSERT_X(false, "NServer::jobToString", "Conversion missing.");
			return "";
			break;
	}	
}
