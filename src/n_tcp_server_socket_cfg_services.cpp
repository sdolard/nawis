#include <QScriptEngine>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_tcp_server_socket_auth_services.h"

#include "n_tcp_server_socket_cfg_services.h"

NTcpServerSocketCfgServices * NTcpServerSocketCfgServices::m_instance = NULL;

NTcpServerSocketCfgServices & NTcpServerSocketCfgServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketCfgServices();
    return *m_instance;
}

void NTcpServerSocketCfgServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketCfgServices::NTcpServerSocketCfgServices()
{
}

NTcpServerSocketCfgServices::~NTcpServerSocketCfgServices()
{
}

NResponse & NTcpServerSocketCfgServices::sharedDir(const NClientSession & session, NResponse & response)
{
    if (session.request().method() == "POST")
        return postSharedDir(session, response);
    if (session.request().method() == "PUT")
        return putSharedDir(session, response);
    if (session.request().method() == "GET")
        return getSharedDir(session, response);
    if (session.request().method() == "DELETE")
        return deleteSharedDir(session, response);

    Q_ASSERT(false);
    return response;
}

// Configuration
NResponse & NTcpServerSocketCfgServices::getSharedDir(const NClientSession &, NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    NDirList sd = NConfig::instance().sharedDirectories();
    int count = sd.count();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(count > 0));
    svRoot.setProperty(RSP_MSG, QScriptValue(count > 0 ? RSP_MSG_LOADED : RSP_MSG_NO_RESULTS));
    svRoot.setProperty(RSP_COUNT, QScriptValue(count));

    QScriptValue svData = se.newArray(count);
    svRoot.setProperty(RSP_DATA, svData);
    for(int i = 0; i < count; i++)
    {
        NDir dir = sd.at(i);
        QScriptValue svDir = se.newObject();
        svData.setProperty(i, svDir);
        svDir.setProperty("id", i);
        svDir.setProperty("path", dir.path());
        svDir.setProperty("recursive", dir.recursive());
        svDir.setProperty("shared", dir.shared());
        svDir.setProperty("exists", dir.exists());
    }
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketCfgServices::postSharedDir(const NClientSession & session, NResponse & response)
{
    //logDebug("NTcpServerSocketCfgServices::svcPostSharedDir", session.postData());

    QScriptEngine se;
    se.evaluate("var data = " + QString::fromUtf8(session.content()));
    QScriptValue svReadData = se.globalObject().property("data").property("data");
    NDir dir = NDir(svReadData.property("path").toString(),
                    svReadData.property("recursive").toBool(),
                    svReadData.property("shared").toBool());

    int id  = getConfig().addSharedDirectory(dir);

    QScriptValue svRoot = se.newObject();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_LOADED));

    // we add new user to response
    QScriptValue svData = se.newArray(1);
    svRoot.setProperty(RSP_DATA, svData);
    QScriptValue svDir = se.newObject();
    svData.setProperty(0, svDir);
    svDir.setProperty("id", id);
    svDir.setProperty("path", dir.path());
    svDir.setProperty("recursive", dir.recursive());
    svDir.setProperty("shared", dir.shared());
    svDir.setProperty("exists", dir.exists());
    //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketCfgServices::putSharedDir(const NClientSession & session, NResponse & response)
{
    //logDebug("NTcpServerSocketCfgServices::svcPutSharedDir", session.postData());
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    QString strId = session.resource();
    logDebug("svcPutSharedDir strId", strId);
    bool ok;
    int id = strId.toInt(&ok);
    if (!ok)
    {
        svRoot.setProperty(RSP_SUCCESS, QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX_PROPERTY));
        //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    se.evaluate("var data = " + QString::fromUtf8(session.content()));
    if (se.hasUncaughtException()){
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_JSON));
        logDebug("NTcpServerSocketCfgServices::svcPutSharedDir", se.uncaughtExceptionBacktrace());
        logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    NDirList sharedDirs = getConfig().sharedDirectories();
    if (id >= sharedDirs.count())
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX));
        //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    QScriptValue svReadDir = se.globalObject().property("data").property("data");
    QString path = svReadDir.property("path").toString();
    QString recursive = svReadDir.property("recursive").toString();
    QString shared = svReadDir.property("shared").toString();

    logDebug("path", path);
    logDebug("recursive", recursive);
    logDebug("shared", shared);

    NDir dir = sharedDirs.at(id);
    NDir modifiedDir = NDir(path.isEmpty() ? dir.path() : path,
                            recursive.isEmpty() ? dir.recursive() : QVariant(recursive).toBool(),
                            shared.isEmpty() ? dir.shared() : QVariant(shared).toBool());

    modifiedDir = getConfig().modifySharedDirectory(id, modifiedDir);
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue(QString(RSP_MSG_N_UPDATED).arg(id)));
    QScriptValue svData = se.newArray();
    svRoot.setProperty(RSP_DATA, svData);
    QScriptValue svDir = se.newObject();
    svData.setProperty(0, svDir);
    svDir.setProperty("id", id);
    svDir.setProperty("path", modifiedDir.path());
    svDir.setProperty("recursive", modifiedDir.recursive());
    svDir.setProperty("shared", modifiedDir.shared());
    svDir.setProperty("exists", modifiedDir.exists());
    //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketCfgServices::deleteSharedDir(const NClientSession & session, NResponse & response)
{
    logDebug("NTcpServerSocketCfgServices::svcDeleteSharedDir", session.resource());
    int id = 0;
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QString strId = session.resource();
    bool ok;
    id = strId.toInt(&ok);
    if (!ok)
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX));
        //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }

    NDirList sharedDirs = getConfig().sharedDirectories();
    if (id >= sharedDirs.count())
    {
        svRoot.setProperty(RSP_SUCCESS , QScriptValue(false));
        svRoot.setProperty(RSP_MSG, QScriptValue(RSP_MSG_INVALID_INDEX));
        //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
        response.setData(NJson::serializeToQByteArray(svRoot));
        return response;
    }
    getConfig().removeSharedDirectory(id);

    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue(QString(RSP_MSG_N_DELETED).arg(id)));
    //logDebug("NJson::serialize(svRoot)", NJson::serialize(svRoot));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}


NResponse & NTcpServerSocketCfgServices::lookForModification(NResponse & response)
{
    getConfig().clearDirUpdateData();
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue("Server will looking for modification"));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketCfgServices::getAvailableLevelList(NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();

    QStringList levels  = NTcpServerAuthSession::toStringLevel(AUTH_LEVEL_ADMIN, " ").split(" ");

    svRoot.setProperty(RSP_SUCCESS , QScriptValue(levels.count() > 0));
    svRoot.setProperty(RSP_MSG, QScriptValue(levels.count() > 0 ? RSP_MSG_LOADED : RSP_MSG_NO_RESULTS));
    svRoot.setProperty(RSP_COUNT, QScriptValue(levels.count()));

    QScriptValue svData = se.newArray(levels.count());
    svRoot.setProperty(RSP_DATA, svData);
    for(int i = 0; i < levels.count(); i++)
    {
        QScriptValue svLevel = se.newObject();
        svData.setProperty(i, svLevel);
        svLevel.setProperty("id", NTcpServerAuthSession::toIntLevel(levels.at(i)));
        svLevel.setProperty("level", levels.at(i));
    }
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}
