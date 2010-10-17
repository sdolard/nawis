
#include <QScriptEngine>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_database.h"

#include "n_tcp_server_socket_auth_services.h"

NTcpServerSocketAuthServices * NTcpServerSocketAuthServices::m_instance = NULL;

NTcpServerSocketAuthServices & NTcpServerSocketAuthServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketAuthServices();
    return *m_instance;
}

void NTcpServerSocketAuthServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketAuthServices::NTcpServerSocketAuthServices()
{
}

NTcpServerSocketAuthServices::~NTcpServerSocketAuthServices()
{
}

void NTcpServerSocketAuthServices::removeExpiredSession() {
    m_authSessionHash.removeExpired();
}

const NTcpServerAuthSession NTcpServerSocketAuthServices::getSession(const QString & sessionId) const
{
    return m_authSessionHash.value(sessionId);
}

bool NTcpServerSocketAuthServices::isSessionValid(const NClientSession & session, int requiredLevel)
{
    return m_authSessionHash.isValid(session, requiredLevel);
}

bool NTcpServerSocketAuthServices::isSessionValid(const QString & sessionId, const QString address,
             const QString & userAgent, int requiredLevel)
{
    return m_authSessionHash.isValid(sessionId, address, userAgent, requiredLevel);
}


NResponse & NTcpServerSocketAuthServices::auth(const NClientSession & session,
                                               NResponse & response)
{
    if (session.request().method() == "POST") {
        return postAuth(session, response);
    }

    if (session.request().method() == "DELETE") {
        return deleteAuth(session, response);
    }

    if (session.request().method() == "GET")
    {
        // POST, GET, DELETE, PUT, END > five elements in array
        NService_n::NService services[5];
        session.getServices(services);
        return getHelp(services, response);
    }

    Q_ASSERT(false);
    return response;
}

NResponse & NTcpServerSocketAuthServices::postAuth(const NClientSession & session, NResponse & response)
{
    // TODO: do automated test

    QScriptEngine se;
    QString login;
    QString password;
    // "json" contentType
    if (session.request().contentType() == NMimeType_n::fileSuffixToMIME("json")){
        QScriptValue svReadRoot = se.newObject();
        se.evaluate("var data = " + QString::fromUtf8(session.content()));
        if (se.hasUncaughtException()){
            svReadRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
            svReadRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_JSON));
            logDebug("NTcpServerSocketAuthServices::postAuth", se.uncaughtExceptionBacktrace());
            logDebug("NJson::serialize(svRoot)", NJson::serialize(svReadRoot));
            response.setData(NJson::serializeToQByteArray(svReadRoot));
            return response;
        }
        QScriptValue svReadData = se.globalObject().property("data").property("data");
        login = svReadData.property("username").toString();
        password = svReadData.property("password").toString();
    }
    // "form" contentType
    if (session.request().contentType() == NMimeType_n::fileSuffixToMIME("form"))
    {
        const NStringMap data = session.contentToMap();
        login = data["username"];
        password = data["password"];
    }


    int level = AUTH_LEVEL_NONE;
    bool authSucceed = false;
    if (getConfig().AdminUser() == login) { // Admin auth
        // Admin level login can only work with getConfig().AdminUser() (cf another account with same login)
        if (getConfig().AdminPassword() == password)
        {
            level = AUTH_LEVEL_ADMIN;
            authSucceed = true;
        }
    } else { // User auth

        NStringMap user = NDB.getUserByEmail(login);
        level =  NTcpServerAuthSession::toIntLevel(user["level"]);
        if (level != AUTH_LEVEL_NONE) {
            authSucceed = user.count() > 0 &&
                          user["password"] == getConfig().toPasswordHash(password) &&
                          QVariant(user["enabled"]).toBool();
        }
    }

    QScriptValue svRoot = se.newObject();
    if (!authSucceed){
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue("Authentication failed"));

        logMessage("Authentication login failed", QString("%1@%2; user agent: %3").
                   arg(login).
                   arg(session.peerAddress()).
                   arg(session.userAgent()));
    } else {
        // Some one try to auth many time on the same IP
        // Other browser, IP nat ? So access is granted, but we log
        QString sessionId = m_authSessionHash.sessionId(session.peerAddress(), login);
        if (!sessionId.isEmpty())
        {
            const NTcpServerAuthSession & authSession = m_authSessionHash.value(sessionId);
            logMessage("WARNING: Authentication login while already logged in",
                       QString("From %1@%2; Already logged from %3; level: %4; user agent: %5").
                       arg(login).
                       arg(session.peerAddress()).
                       arg(authSession.address()).
                       arg(NTcpServerAuthSession::toStringLevel(authSession.level())).
                       arg(session.userAgent()));
        }

        // Administrator level
        NTcpServerAuthSession authSession;
        authSession.set(session.peerAddress(), login, session.userAgent(), level);
        m_authSessionHash.insert(authSession.sessionId(), authSession);
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
        svRoot.setProperty(RSP_MSG, QScriptValue("Authentication succeed"));
        svRoot.setProperty("level", QScriptValue(NTcpServerAuthSession::toStringLevel(authSession.level())));
        response.setSessionCookie(authSession.sessionId(), session.isSsl());
        logMessage("Authentication login succeed", QString("%1@%2; level: %3; user agent: %4").
                   arg(authSession.login()).
                   arg(authSession.address()).
                   arg(NTcpServerAuthSession::toStringLevel(authSession.level())).
                   arg(authSession.userAgent()));
    }

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketAuthServices::deleteAuth(const NClientSession & session, NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    if (m_authSessionHash.contains(session.sessionId()))
    {
        const NTcpServerAuthSession & authSession = m_authSessionHash.value(session.sessionId());
        if (authSession.address() == session.peerAddress())
        {
            logMessage("Authentication logout succeed", QString("%1@%2; user agent: %3").
                       arg(authSession.login()).
                       arg(session.peerAddress()).
                       arg(session.userAgent()));
            svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
            svRoot.setProperty(RSP_MSG, QScriptValue(QString("%1 unauthenticated").arg(authSession.login())));
            m_authSessionHash.remove(session.sessionId());
            response.clearSessionCookie();
        } else {
            logMessage("Authentication logout WARNING",
                       QString("try to unauth %1@%2 (authentication address is %3); user agent: %4").
                       arg(authSession.login()).
                       arg(session.peerAddress()).
                       arg(authSession.address()).
                       arg(session.userAgent()));

            svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
            svRoot.setProperty(RSP_MSG, QScriptValue(QString("Try to unauthenticate %1 from a different "\
                                                             "authenticated address. Your Ip is logged.(%2)").
                                                     arg(authSession.login()).
                                                     arg(session.peerAddress())));

        }
    } else {
        logMessage("Authentication logout ERROR", QString("no authentication for %1; user agent: %2").
                   arg(session.peerAddress()).
                   arg(session.userAgent()));
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue("You are not authenticated"));
    }
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}
