#include "n_thread.h"

NThread::NThread(QObject * parent)
	:QThread(parent)
{
	m_stop = false;
}

void NThread::stop()
{
	QMutexLocker locker(&m_stopMutex);
	m_stop = true; 
}

bool NThread::isStopping()
{
	QMutexLocker locker(&m_stopMutex); 
	return m_stop;
}
