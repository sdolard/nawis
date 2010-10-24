#ifndef N_TCP_SERVER_AUTH_SESSION_H
#define N_TCP_SERVER_AUTH_SESSION_H

// Qt
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QNetworkCookie>
#include <QHash>

#include "n_client_session.h"

#define AUTH_LEVEL_NONE           0x0000
// Full levels
#define AUTH_LEVEL_ADMIN          0xFFFF

// Required to access to download api
#define AUTH_LEVEL_DOWNLOAD       0x0001
// Required to access to duplicated api
#define AUTH_LEVEL_DUPLICATED     0x0002
// Required to access to log api
#define AUTH_LEVEL_LOG            0x0004
// Required  to access to music api
#define AUTH_LEVEL_MUSIC          0x0008
// Required  to access to picture api
#define AUTH_LEVEL_PICTURE        0x0010
// Required  to access to search api
#define AUTH_LEVEL_SEARCH         0x0020


class NTcpServerAuthSession
{
public: 

    NTcpServerAuthSession();

    // Five minutes ( 300 s )
    bool isExpired() const;

    // reset used timer
    void setUsed();

    const QString sessionId() const;
    const QString address() const;
    const QString login() const;
    const QString userAgent() const;

    int level() const;
    bool isLevelSet(int level) const;
    // Return levels space separated
    static const QString toStringLevel(int levels, const QString & sep = " ");
    static int toIntLevel(const QString & levels, const QString & sep = " ");

    // Cookie is generated here
    void set(const QString & address, const QString & login, const QString & userAgent, int level);

private:
    QDateTime m_lastUsed;
    QString   m_address;
    QString   m_login;
    QString   m_sessionId;
    int       m_level;
    QString   m_userAgent;

};

// TODO: limit session number?
class NTcpServerAuthSessionHash: public QHash<QString, NTcpServerAuthSession>
{
public:
    const QString sessionId(const QString & address, const QString & login) const;
    void removeExpired();

    bool isValid(const NClientSession & session, int requiredLevel);

    bool isValid(const QString & sessionId, const QString address,
                 const QString & userAgent, int requiredLevel);
};

#endif // N_TCP_SERVER_AUTH_SESSION_H

