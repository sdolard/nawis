#include "n_server.h"
#include "n_log.h"
#include "n_log_database.h"

#include "n_nawis_daemon.h"

NDaemon::NDaemon(int argc, char **argv)
    : QtService<QtSingleCoreApplication>(argc, argv, "Nawis daemon")
{
    setServiceDescription("Nawis daemon");
}


NDaemon::~NDaemon()
{
}

void NDaemon::start()
{
    QtSingleCoreApplication *app = application();
    if (app->isRunning()) {
        QString msg(QString("Hi master, I am %1.").arg(QCoreApplication::applicationPid()));
        bool sentok = app->sendMessage(msg);
        QString rep("Another instance is running, so I will exit.");
        rep += sentok ? " Message sent ok." : " Message sending failed.";
        qDebug("NDaemon::start: %s", qPrintable(rep));
        app->quit();
        return;
    } else {
        //qDebug("NDaemon::start: %s",  "No other instance is running; so I will.");
        QObject::connect(app, SIGNAL(messageReceived(const QString&)),
                         &m_singleAppMsgHandler, SLOT(newMessage(const QString&)));
    }

    app->setApplicationName("nawis");
    app->setOrganizationDomain("nawis");
    app->setOrganizationName("nawis");

    NLog::start(); // log thead init

    NLOGM("Server", "--------------- new session ---------------");

    if (!NSERVER.start())
        app->quit();
}

void NDaemon::stop()
{
    NSERVER.stop();
}

void NDaemon::pause()
{
    NSERVER.pause();
}

void NDaemon::resume()
{
    NSERVER.resume();
}

void NDaemon::processCommand(int code)
{
    NLOGDD("processCommand", QString::number(code));
}


/*****************************************
 * NSingleAppMsgHandler
 ****************************************/
NSingleAppMsgHandler::NSingleAppMsgHandler()
    : QObject()
{
}


void NSingleAppMsgHandler::newMessage(const QString & message)
{
    NLOGD("NSingleAppMsgHandler::newMessage", message);
}
