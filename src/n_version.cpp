#include <QApplication>

#include "n_version.h"


const QString NVersion_n::namedVersion(bool os)
{
    Q_ASSERT(qApp);
    QString nawis = QString("%1/%2.%3 ").arg(qApp->applicationName()).arg(getNawisVersion()).arg(getNawisBuild());
    if (os) {
#ifdef Q_OS_DARWIN 
	nawis += "(Macintosh)";
#elif defined(Q_OS_WIN32)
	nawis += "(Windows)";
#elif defined(Q_OS_LINUX)
	nawis += "(Linux)";
#endif
    }
    return nawis.trimmed();
}

