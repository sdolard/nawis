// Qt
#include <QDir>
#include <QApplication>


// Windows 
#ifdef Q_OS_WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <tchar.h>
#endif

// App
#include "n_compat.h"

#include "n_path.h"

#ifdef Q_OS_UNIX
QString unixPath()
{
    if (QDir::homePath() == QDir::rootPath()) {
        return "/etc";
    }
    return QString("%1/.config").arg(QDir::homePath());
}
#endif // Q_OS_UNIX

#ifdef Q_OS_WIN32
QString windowsPath()
{
    TCHAR szPath[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPath(NULL,
                                 CSIDL_APPDATA,
                                 NULL,
                                 0,
                                 szPath)))
    {
        return QTBASE_TCHAR_TO_QSTRING(szPath);
    }
    Q_ASSERT(false);
    qFatal("An error occured");
    return "";
}
#endif //Q_OS_UNIX

QString NPath_n::config()
{
    Q_ASSERT(!qApp->organizationDomain().isEmpty());

    QString appDataPath;
#if defined(Q_OS_UNIX) // MAC included
    appDataPath = unixPath();
#elif defined(Q_OS_WIN32)
    appDataPath = windowsPath();
#else
    TODO // For other platform
#endif //

            QString path = QDir::toNativeSeparators(QString("%1/%2/").
                                                    arg(appDataPath).
                                                    arg(qApp->organizationDomain()));
    //qDebug("Application configuration path: %s", qPrintable(path));
    return path;
}
