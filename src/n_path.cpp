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

QString NPath_n::userConfig()
{
	Q_ASSERT(!qApp->organizationDomain().isEmpty());
	QString appDataPath;
#if defined(Q_OS_UNIX) // MAC included
	appDataPath = QString("%1/.config").arg(QDir::homePath());
#elif defined(Q_OS_WIN32)
	TCHAR szPath[MAX_PATH];
	if(SUCCEEDED(SHGetFolderPath(NULL,
								 CSIDL_APPDATA,
								 NULL,
								 0,
								 szPath)))
		appDataPath = QTBASE_TCHAR_TO_QSTRING(szPath);
	
#else
	TO DO // For other platform
#endif	
	return QDir::toNativeSeparators(QString("%1/%2/").
									arg(appDataPath).
									arg(qApp->organizationDomain()));
}
