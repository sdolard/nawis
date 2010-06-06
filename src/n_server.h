#ifndef N_SERVER_H
#define N_SERVER_H

// Qt
#include <QObject> 
#include <QString>      
#include <QTimer>

// App
#include "n_tcp_server.h"
#include "n_db_updater.h"
#include "n_hasher_thread.h"
#include "n_metadata_updater_thread.h"
#include "n_music_db_updater_thread.h"

#define NSERVER (NServer::instance())

// Order is important !
#define JT_NONE            0x00
#define JT_WATCH_FILES     0X01
#define JT_DB_UPDATE       0x02
#define JT_HASH            0x03
#define JT_GET_METADATA    0x04
#define JT_BUILD_MUSIC_DB  0x05
#define JT_LAST            JT_BUILD_MUSIC_DB

class NServer: public QObject
{
    Q_OBJECT
public:
    static NServer & instance(QObject *parent = NULL/* first instance params must be application*/);
    static void deleteInstance();

    ~NServer();

    bool start();
    bool stop();
    bool pause();
    bool resume();

    int jobStatus() const;
    static const QString jobToString(int job);

public slots:
    void onConfigFileChanged();

private:
    NServer(QObject *parent);

    static NServer           *m_instance;
    int                       m_currentJob;
    NTcpServer               *m_server;
    NTcpServer               *m_sslServer;
    NDirWatcherThreadItems    m_dirs;
    NDbUpdaterThread         *m_dbUpdaterJob;
    NDirWatcherThread        *m_dirWatcherJob;
    NHasherThread            *m_hasherJob;
    NMetadataUpdaterThread   *m_metadataUpdaterJob;
    NMusicDbUpdaterThread    *m_musicDbUpdaterJob;
    QTimer                    m_jobTimer;
    NDirList                  m_sharedDirectories;
    NFileSuffixList           m_fileSuffixes;
    bool                      m_configFileChanged;

    // TCP
    bool startTcpServer();
    bool restartTcpServer();
    void stopTcpServer();

    // TCP SSL
    bool startSslTcpServer();
    bool restartSslTcpServer();
    void stopSslTcpServer();

    void stopJobs();
    void stopJob(int job);
    void startJobs();
    void startJob(int job);

    QString socketErrorToString(QAbstractSocket::SocketError error);

private slots:
    void onJobTimerTimeout();
    void onDirWatcherHash(QString hash, NDirWatcherThreadItems dirs);
    void onJobTerminated();
};

#endif // N_SERVER_H

