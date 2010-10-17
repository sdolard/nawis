#ifndef N_THREAD_H
#define N_THREAD_H

// Qt
#include <QThread>
#include <QMutex>

class NThread: public QThread
{
    Q_OBJECT
public: 
    NThread(QObject * parent = 0);
    virtual ~NThread();

    void stop();

protected:
    bool isStopping();

private:
    bool     m_stop;
    QMutex   m_stopMutex;
};
#endif // N_THREAD_H
