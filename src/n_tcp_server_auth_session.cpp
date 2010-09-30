// Qt
#include <QUuid>

// App
#include "n_log.h"
#include "n_config.h"

#include "n_tcp_server_auth_session.h"

NTcpServerAuthSession::NTcpServerAuthSession()
{
}

bool NTcpServerAuthSession::isExpired() const
{
    int lastActivity = m_lastUsed.secsTo ( QDateTime::currentDateTime() );
    if (lastActivity >= 300) {
        logMessage("Authentication expired", QString("%1@%2; user agent: %3").
              arg(m_login).
              arg(m_address).
              arg(m_userAgent));
        return true;
    }
    return false;
}

void NTcpServerAuthSession::setUsed()
{
    if (!isExpired())
        m_lastUsed = QDateTime::currentDateTime();
}

const QString NTcpServerAuthSession::sessionId() const
{
    return m_sessionId;
}

const QString NTcpServerAuthSession::address() const
{
    return m_address;
}

const QString NTcpServerAuthSession::login() const
{
    return m_login;
}

const QString NTcpServerAuthSession::userAgent() const
{
    return m_userAgent;
}

void NTcpServerAuthSession::set(const QString & address, const QString & login,
                                const QString & userAgent, int level)
{
    m_address = address;
    m_login = login;
    m_level = level;
    m_sessionId = QUuid::createUuid().toString().remove("{").remove("}");
    m_lastUsed = QDateTime::currentDateTime();
    m_userAgent = userAgent;
}

int NTcpServerAuthSession::level() const
{
    return m_level;
}

bool NTcpServerAuthSession::isLevelSet(int level) const
{
    return (m_level & level) == level;
}

const QString NTcpServerAuthSession::levelToString(int level)
{
    QStringList levelList;
    if ((level & AUTH_LEVEL_USER) == AUTH_LEVEL_USER)
        levelList << "user";

    if ((level & AUTH_LEVEL_ADMIN) == AUTH_LEVEL_ADMIN)
        levelList << "admin";

    return levelList.join(" ");
}


/*******************************************************************************
 NTcpServerAuthSessionHash
 *******************************************************************************/
const QString NTcpServerAuthSessionHash::sessionId(const QString & address, const QString & login) const
{
    QHash<QString, NTcpServerAuthSession>::const_iterator i = constBegin();
    while (i != constEnd()) {
        if (i.value().address() == address &&
            i.value().login() == login)
            return i.key();
        ++i;
    }
    return "";
}

void NTcpServerAuthSessionHash::removeExpired()
{
    QStringList toRemove;
    QHash<QString, NTcpServerAuthSession>::const_iterator i = constBegin();
    while (i != constEnd()) {
        if (i.value().isExpired())
            toRemove.append(i.key());
        ++i;
    }
    while (toRemove.count() > 0)
        remove(toRemove.takeFirst());
}

bool NTcpServerAuthSessionHash::isValid(const NClientSession & session, int requiredLevel)
{
    return isValid(session.sessionId(), session.peerAddress(), session.userAgent(), requiredLevel);
}

bool NTcpServerAuthSessionHash::isValid(const QString & sessionId, const QString address,
                                        const QString & userAgent, int requiredLevel)
{
    if (!contains(sessionId))
        return false;

    NTcpServerAuthSession & authSession = (*this)[sessionId];
    if (authSession.address() != address)
        return false;
    if (authSession.userAgent() != userAgent)
        return false;

    authSession.setUsed();

    return authSession.isLevelSet(requiredLevel);
}
