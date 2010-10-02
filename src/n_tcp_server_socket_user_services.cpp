#include <QHostAddress>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_convert.h"
#include "n_tcp_server_socket_auth_services.h"
#include "n_database.h"

#include "n_tcp_server_socket_user_services.h"

NTcpServerSocketUserServices * NTcpServerSocketUserServices::m_instance = NULL;

NTcpServerSocketUserServices & NTcpServerSocketUserServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketUserServices();
    return *m_instance;
}

void NTcpServerSocketUserServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketUserServices::NTcpServerSocketUserServices()
{
}

NTcpServerSocketUserServices::~NTcpServerSocketUserServices()
{
}


NResponse & NTcpServerSocketUserServices::user(const NClientSession & session, NResponse & response)
{
    if (session.request().method() == "POST") {
        return postUser(session, response);
    }

    if (session.request().method() == "PUT") {
        return putUser(session, response);
    }

    if (session.request().method() == "GET") {
        return getUser(session, response);
    }

    if (session.request().method() == "DELETE") {
        return deleteUser(session, response);
    }

    Q_ASSERT(false);
    return response;
}

NResponse & NTcpServerSocketUserServices::getUser(const NClientSession & session, NResponse & response)
{
    bool ok;
    int start = session.url().queryItemValue("start").toInt();
    if (!ok)
        start = 0;
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    QString sort = session.url().queryItemValue("sort");
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for users: \"%2\"; start: %3; limit: %4, sort:\"%5\", dir:\"%6\"").
         arg(authSession.login()). // 1
         arg(NConvert_n::fromUTF8PercentEncoding(search)).// 2
         arg(start).// 3
         arg(limit).// 4
         arg(sort).// 5
         arg(dir));// 6

    int totalCount = NDB.getUserListCount(searches);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);

    bool succeed = NDB.getUserList(se, svData, searches, start, limit, sort, dir);

    setJsonRootReponse(svRoot, totalCount, succeed);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketUserServices::postUser(const NClientSession & session, NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    se.evaluate("var data = " + QString::fromUtf8(session.content()));
    if (se.hasUncaughtException()){
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_JSON));
        logDebug("NTcpServerSocketUserServices::svcPostUser", se.uncaughtExceptionBacktrace());
        logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    QScriptValue svReadUser = se.globalObject().property("data").property("data");
    QString email = svReadUser.property("email").toString();
    QString name = svReadUser.property("name").toString();
    QString password = NCONFIG.toPasswordHash(svReadUser.property("password").toString());

    logDebug("email", email);
    logDebug("name", name);
    logDebug("password", password);

    // User already exists?
    NStringMap user = NDB.getUserByEmail(email);

    if (user.count() > 0) // User already exists
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, "User already exists");
        logMessage("Registration failed", QString("already registered; %1(%2); user agent: %3").
              arg(email).
              arg(session.peerAddress()).
              arg(session.userAgent()));

        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    // User do not exists
    int userId = NDB.addUser(name, email, password);
    if (userId < 0) // Error
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        QString errorMsg;
        switch(userId){
        case DB_USER_ERROR_INVALID_PARAMS:
            errorMsg = "Invalid params";
            break;
        case  DB_USER_ERROR_QUERY:
            errorMsg = "Query error";
            break;
        }
        svRoot.setProperty(RSP_MSG, errorMsg);
        logMessage("User add failed", QString("%1; %2(%3); user agent: %4").
              arg(errorMsg).
              arg(email).
              arg(session.peerAddress()).
              arg(session.userAgent()));

        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QString("User %1 added (account not enabled").arg(email));
    logMessage("User add succeed", QString("%1(%2); user agent: %3").
          arg(email).
          arg(session.peerAddress()).
          arg(session.userAgent()));

    // We add new user to response
    QScriptValue svData = se.newArray(1);
    svRoot.setProperty(RSP_DATA, svData);
    QScriptValue svUser = se.newObject();
    svData.setProperty(0, svUser);
    svUser.setProperty("id", userId);
    svUser.setProperty("name", name);
    svUser.setProperty("email", email);
    svUser.setProperty("enabled", false);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketUserServices::putUser(const NClientSession & session, NResponse & response)
{
    //logDebug("NTcpServerSocketUserServices::svcPutSharedDir", session.postData());
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    QString strId = session.resource();
    logDebug("svcPutUser strId", strId);
    bool ok;
    int id = strId.toInt(&ok);
    if (!ok)
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_USER));
        //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    se.evaluate("var data = " + QString::fromUtf8(session.content()));
    if (se.hasUncaughtException()){
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_JSON));
        logDebug("NTcpServerSocketUserServices::svcPutUser", se.uncaughtExceptionBacktrace());
        logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    NStringMap user = NDB.getUserById(id);
    if (user.count()  == 0)
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_USER));
        //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    QScriptValue svReadUser = se.globalObject().property("data").property("data");
    QString email = svReadUser.property("email").toString();
    QString password = NCONFIG.toPasswordHash(svReadUser.property("password").toString());
    QString name = svReadUser.property("name").toString();
    QString preferences = svReadUser.property("preferences").toString();
    QString enabled = svReadUser.property("enabled").toString();
    QString level = svReadUser.property("level").toString();

    email = email.isEmpty() ? user["email"] : email;
    password = password.isEmpty() ? user["password"] : password;
    name = name.isEmpty() ? user["name"] : name;
    preferences = preferences.isEmpty() ? user["preferences"] : preferences;
    enabled = enabled.isEmpty() ? user["enabled"] : enabled;
    level = level.isEmpty() ? user["level"] : level;

    logDebug("email", email);
    logDebug("password", password);
    logDebug("name", name);
    logDebug("preferences", preferences);
    logDebug("enabled", enabled);
    logDebug("level", level);

    if (!NDB.updateUser(id, email, password, name, preferences,
                        QVariant(enabled).toBool(),level)) {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_ERROR_OCCURRED));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue(QString(RSP_MSG_N_UPDATED).arg(id)));
    QScriptValue svData = se.newArray();
    svRoot.setProperty(RSP_DATA, svData);
    QScriptValue svDir = se.newObject();
    svData.setProperty(0, svDir);
    svDir.setProperty("id", id);
    svDir.setProperty("email", email);
    svDir.setProperty("name", name);
    svDir.setProperty("preferences", preferences);
    svDir.setProperty("enabled", QVariant(enabled).toBool());
    svDir.setProperty("level", level);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketUserServices::deleteUser(const NClientSession & session, NResponse & response)
{
    logDebug("NTcpServerSocketUserServices::svcDeleteUser", session.resource());
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QString id = session.resource();
    bool userDeleted =  NDB.deleteUser(id);
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(userDeleted));
    svRoot.setProperty(RSP_MSG, userDeleted ?
                       QScriptValue(QString("User %1 deleted").arg(id)) :
                       QScriptValue(QString("User %1 not deleted").arg(id)));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}
