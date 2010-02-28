#ifndef N_NAWIS_DAEMON_H
#define N_NAWIS_DAEMON_H

#include <QCoreApplication>
#include <QObject>

#include <qtservice.h>
#include <qtsinglecoreapplication.h>

class NSingleAppMsgHandler: public QObject
{
    Q_OBJECT
public:
    NSingleAppMsgHandler();

public slots:
    void newMessage(const QString& message);
};


class NDaemon : public QtService<QtSingleCoreApplication>
{
public:
    NDaemon(int argc, char **argv);
    ~NDaemon();

protected:
    void start();
    void stop();
    void pause();
    void resume();
    void processCommand(int code);

private:
    NSingleAppMsgHandler m_singleAppMsgHandler;
};

#endif // N_NAWIS_DAEMON_H
