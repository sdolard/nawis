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

const QString NTcpServerAuthSession::toStringLevel(int levels, const QString & sep)
{
    QStringList levelList;
    if (levels == AUTH_LEVEL_ADMIN)
        levelList << "admin";

    if ((levels & AUTH_LEVEL_DOWNLOAD) == AUTH_LEVEL_DOWNLOAD)
        levelList << "download";

    if ((levels & AUTH_LEVEL_DUPLICATED) == AUTH_LEVEL_DUPLICATED)
        levelList << "duplicated";

    if ((levels & AUTH_LEVEL_LOG) == AUTH_LEVEL_LOG)
        levelList << "log";

    if ((levels & AUTH_LEVEL_MUSIC) == AUTH_LEVEL_MUSIC)
        levelList << "music";

    if ((levels & AUTH_LEVEL_PICTURE) == AUTH_LEVEL_PICTURE)
        levelList << "picture";

    if ((levels & AUTH_LEVEL_SEARCH) == AUTH_LEVEL_SEARCH)
        levelList << "search";

    return levelList.join(sep);
}

int NTcpServerAuthSession::toIntLevel(const QString & levels, const QString & sep)
{
    int intLevel = AUTH_LEVEL_NONE;
    if (levels.isEmpty()) {
        return intLevel;
    }

    QStringList l = levels.toLower().split(sep);
    for(int i = 0; i < l.count(); i++)
    {
        const QString & level  = l.at(i);
        if (level == "admin") {
            /*
                AUTH_LEVEL_ADMIN is reserved to "admin" special user,
                so it's not possible to require explicitly this level
            */
            continue;
        }

        if (level == "download")
        {
            intLevel |= AUTH_LEVEL_DOWNLOAD;
            continue;
        }

        if (level == "duplicated")
        {
            intLevel |= AUTH_LEVEL_DUPLICATED;
            continue;
        }
        if (level == "log")
        {
            intLevel |= AUTH_LEVEL_LOG;
            continue;
        }
        if (level == "music")
        {
            intLevel |= AUTH_LEVEL_MUSIC;
            continue;
        }
        if (level == "picture")
        {
            intLevel |= AUTH_LEVEL_PICTURE;
            continue;
        }
        if (level == "search")
        {
            intLevel |= AUTH_LEVEL_SEARCH;
            continue;
        }

        Q_ASSERT_X(false, "NTcpServerAuthSession::toIntLevel: not managed level?", qPrintable(levels));
    }
    return intLevel;
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
