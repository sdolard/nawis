#ifndef N_TCP_SERVER_AUTH_SESSION_H
#define N_TCP_SERVER_AUTH_SESSION_H

// Qt
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QNetworkCookie>
#include <QHash>

#include "n_client_session.h"

#define AUTH_LEVEL_NONE      0x00
#define AUTH_LEVEL_USER      0x01
#define AUTH_LEVEL_ADMIN     0x02

#define AUTH_LEVEL_ALL       0x99

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
    static const QString levelToString(int level);

    // Cookie is generated here
    void set(const QString & address, const QString & login, const QString & userAgent, int level);

private:
    QDateTime                     m_lastUsed;
    QString                       m_address;
    QString                       m_login;
    QString                       m_sessionId;
    int                           m_level;
    QString                       m_userAgent;

};

class NTcpServerAuthSessionHash: public QHash<QString, NTcpServerAuthSession>
{
public:
    const QString sessionId(const QString & address, const QString & login) const;
    void removeExpired();

    bool isValid(const NClientSession & session, int requiredLevel = AUTH_LEVEL_USER);

    bool isValid(const QString & sessionId, const QString address,
                 const QString & userAgent, int requiredLevel = AUTH_LEVEL_USER);
};

#endif // N_TCP_SERVER_AUTH_SESSION_H

