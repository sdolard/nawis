#include <QApplication>

#include "n_version.h"


const QString NVersion_n::namedVersion(bool os)
{
    Q_ASSERT(qApp);
    QString nawis = QString("%1/%2.%3 ").arg(qApp->applicationName()).arg(NAWIS_VERSION).arg(NAWIS_BUILD);
    if (os) {
#ifdef Q_OS_MAC
	nawis += "(Macintosh)";
#elif defined(Q_OS_WIN32)
	nawis += "(Windows)";
#elif defined(Q_OS_LINUX)
	nawis += "(Linux)";
#else
        nawis += "(?)";
#endif
    }
    return nawis.trimmed();
}

