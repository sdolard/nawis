#include <QScriptEngine>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_server.h"
#include "n_date.h"
#include "n_http.h"
#include "n_tcp_server_socket_auth_services.h"

#include "n_tcp_server_socket_ui_services.h"

NTcpServerSocketUIServices * NTcpServerSocketUIServices::m_instance = NULL;

NTcpServerSocketUIServices & NTcpServerSocketUIServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketUIServices();
    return *m_instance;
}

void NTcpServerSocketUIServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketUIServices::NTcpServerSocketUIServices()
{
}

NTcpServerSocketUIServices::~NTcpServerSocketUIServices()
{
}

NResponse & NTcpServerSocketUIServices::nop(NResponse & response)
{
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue("Nop"));
    svRoot.setProperty("status", QScriptValue(NServer::jobToString(NSERVER.jobStatus())));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketUIServices::lookForModification(NResponse & response)
{
    getConfig().clearDirUpdateData();
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    svRoot.setProperty(RSP_SUCCESS , QScriptValue(true));
    svRoot.setProperty(RSP_MSG, QScriptValue("Server will looking for modification"));
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketUIServices::getFavicon(NResponse & response)
{
    response.httpHeader().setContentType("image/x-icon");
    response.removeDefaultCharset();

    QFile file(":/images/nawis.ico");
    if (!file.open(QIODevice::ReadOnly))
        return response;
    response.setData(file.readAll());
    response.add10yExpiresHttpHeader();
    return response;
}

NResponse & NTcpServerSocketUIServices::redirectToTUI(int *statusCode,
                                                       const NClientSession & session, NResponse & response)
{
    *statusCode = N_HTTP_MOVED_PERMANENTLY;
    response.httpHeader().setValue("Location", QString("http%1://%2:%3/ui").
                                   arg(session.isSsl() ? "s" : "").
                                   arg(session.host()).
                                   arg(session.port()));
    response.add10yExpiresHttpHeader();
    return response;
}

NResponse & NTcpServerSocketUIServices::getUI(int *statusCode,
                                               const NClientSession & session, NResponse & response)
{
    QString filename("/index.html");
    QString uiPath = filename;
    QString path = session.urlPath();
    //logDebug("path", path);

    path.remove(0, 3); // We remove wanted "/ui" begin path chars
    if (!path.isEmpty())
    {
        filename = path;
        uiPath = filename;
    }
    //logDebug("uiPath", uiPath);

    // Directory traversal test
    filename.prepend(getConfig().serverPub().absolutePath()); // to local file
    QFileInfo fi(filename);
    if (!fi.absoluteFilePath().startsWith(getConfig().serverPub().absolutePath()))
    {
        *statusCode = N_HTTP_FORBIDDEN;
        return response;
    }

    // Redirection to index.html file of dir
    if (fi.isDir())
    {
        *statusCode = N_HTTP_MOVED_PERMANENTLY;
        if (uiPath.endsWith("/"))
            uiPath.remove(path.length() - 1, 1);
        response.httpHeader().setValue("Location", QString("http%1://%2:%3/ui%4/index.html").
                                       arg(session.isSsl() ? "s" : "").
                                       arg(session.host()).
                                       arg(session.port()).
                                       arg(uiPath));
        response.add10yExpiresHttpHeader();
        return response;
    }

    QString fileSuffix = fi.suffix();

    // Redirection to ui/index.hlml if not authenticated
    //logDebug("fileSuffix", fileSuffix);
    //logDebug("nawis_sessionid", session.sessionId());
    //logDebug("path", path);
    //logDebug("m_authSessionHash.isValid(session)", QVariant(m_authSessionHash.isValid(session)).toString());
    if (fileSuffix == "html" &&
        uiPath != "/index.html" &&
        !getAuthServices().isSessionValid(session))
    {
        return redirectToTUI(statusCode, session, response);
    }

    QString gzippedFilename = filename + ".gz";
    QFileInfo gzipFi(gzippedFilename);
    //logDebug("NTcpServerSocketUIServices::svcUI filename", filename);
    if (session.supportCompression(NCompress_n::ctGZip) && gzipFi.exists())
    {
        fi = gzipFi;
        filename = gzippedFilename;
    } else if (!fi.exists()) {
        *statusCode = N_HTTP_NOT_FOUND;
        return response;
    }

    response.httpHeader().setContentType(NMimeType_n::fileSuffixToMIME(fileSuffix));
    response.add10yExpiresHttpHeader();
    response.removeDefaultCharset();

    QString lastModified = NDate_n::toHTMLDateTime(fi.lastModified());
    response.httpHeader().setValue("Last-Modified", lastModified);
    if (session.request().value("If-Modified-Since") == lastModified)
    {
        *statusCode = N_HTTP_NOT_MODIFIED;
        return response;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return response;
    response.setData(file.readAll());
    if (filename == gzippedFilename)
        response.setGZipped();
    return response;
}
