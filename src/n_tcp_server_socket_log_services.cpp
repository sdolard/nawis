#include <QScriptEngine>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_log_database.h"

#include "n_tcp_server_socket_log_services.h"

NTcpServerSocketLogServices * NTcpServerSocketLogServices::m_instance = NULL;

NTcpServerSocketLogServices & NTcpServerSocketLogServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketLogServices();
    return *m_instance;
}

void NTcpServerSocketLogServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketLogServices::NTcpServerSocketLogServices()
{
}

NTcpServerSocketLogServices::~NTcpServerSocketLogServices()
{
}

NResponse & NTcpServerSocketLogServices::log(const NClientSession & session, NResponse & response)
{
    if (session.request().method() == "GET")
        return getLog(session, response);

    if (session.request().method() == "DELETE")
        return deleteLog(response);
    Q_ASSERT(false);
    return response;
}

NResponse & NTcpServerSocketLogServices::deleteLog(NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    bool logCleared = getLogDb().clearLogs();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, logCleared ? "Log cleared" : "Log not cleared");
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketLogServices::getLog(const NClientSession & session,
                                                NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString sort = session.url().queryItemValue("sort");
    QString dir = session.url().queryItemValue("dir");

    int totalCount = getLogDb().getLogListCount(search);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = getLogDb().getLogList(se, svData, search, start, limit, sort, dir);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}
