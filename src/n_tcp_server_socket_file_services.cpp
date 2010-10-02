#include <QScriptEngine>
#include <QHostAddress>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_database.h"
#include "n_convert.h"
#include "n_tcp_server_socket_auth_services.h"
#include "n_http.h"

#include "n_tcp_server_socket_file_services.h"

NTcpServerSocketFileServices * NTcpServerSocketFileServices::m_instance = NULL;

NTcpServerSocketFileServices & NTcpServerSocketFileServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketFileServices();
    return *m_instance;
}

void NTcpServerSocketFileServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketFileServices::NTcpServerSocketFileServices()
{
}

NTcpServerSocketFileServices::~NTcpServerSocketFileServices()
{
}


NResponse & NTcpServerSocketFileServices::search(const NClientSession & session,
                                                   NResponse & response,
                                                   NFileCategory_n::FileCategory category)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString sort = session.url().queryItemValue("sort");
    QString dir = session.url().queryItemValue("dir");

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for file: \"%2\"; category: \"%3\"; start: %4; limit: %5, sort:\"%6\", dir:\"%7\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(category).arg(start).arg(limit).arg(sort).arg(dir));

    int totalCount = NDB.getFileListCount(searches, category);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);


    bool succeed = NDB.getFileList(se, svData, searches, start, limit, category, sort, dir);

    setJsonRootReponse(svRoot, totalCount, succeed);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketFileServices::getDuplicated(const NClientSession & session,
                                                           NResponse & response,
                                                           NFileCategory_n::FileCategory category)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString sort = session.url().queryItemValue("sort");
    QString dir = session.url().queryItemValue("dir");

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for (duplicated) \"%2\"; category: \"%3\"; start: %4; limit: %5, sort:\"%6\", dir:\"%7\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(category).arg(start).arg(limit).arg(sort).arg(dir));

    int totalCount = NDB.getDuplicatedFileListCount(searches, category);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NDB.getDuplicatedFileList(se, svData, searches, start, limit,
                                             category, sort, dir);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketFileServices::download(int *statusCode, const NClientSession & session, NResponse & response)
{
    QString fileHash = session.resource();

    QFileInfo fileInfo = NDatabase::instance().file(fileHash);
    if (!fileInfo.exists())
    {
        *statusCode = N_HTTP_NOT_FOUND;
        return response;
    }
    response.setFileInfo(fileInfo);
    response.httpHeader().setContentType(NMimeType_n::fileSuffixToMIME(fileInfo.suffix()));
    response.removeDefaultCharset();
    return response;
}
