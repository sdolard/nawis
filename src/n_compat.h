#ifndef N_COMPAT_H
#define N_COMPAT_H

#ifdef Q_OS_WIN32
#include <QtGlobal>
#include <QString>
#include <windows.h>
#ifdef UNICODE
#define QTBASE_QSTRING_TO_TCHAR(x)          ((TCHAR*) x.utf16())
#define QTBASE_QSTRING_TO_TCHAR_LEN(x)      (sizeof(TCHAR) * x.length() + 1)
#define QTBASE_PQSTRING_TO_TCHAR(x)         ((TCHAR*) x->utf16())
#define QTBASE_PQSTRING_TO_TCHAR_LEN(x)     (sizeof(TCHAR) * x->length() + 1)
#define QTBASE_TCHAR_TO_QSTRING(x)          QString::fromUtf16((const ushort *)(x))
#define QTBASE_TCHAR_TO_QSTRING_N(x,y)      QString::fromUtf16((const ushort *)(x),(y))
#define QTBASE_TCHAR_BYTE_SIZE(x)           (lstrlen(x) * sizeof(TCHAR))
#else
#define QTBASE_QSTRING_TO_TCHAR(x)          x.toLocal8Bit()
#define QTBASE_PQSTRING_TO_TCHAR(x)         x->toLocal8Bit()
#define QTBASE_TCHAR_TO_QSTRING(x)          QString::fromLocal8Bit((x))
#define QTBASE_TCHAR_TO_QSTRING_N(x,y)      QString::fromLocal8Bit((x),(y))
#endif// UNICODE
#endif// Q_OS_WIN32

#endif //N_COMPAT_H
