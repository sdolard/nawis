#include <QScriptEngine>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_image.h"
#include "n_client_session.h"
#include "n_database.h"

#include "n_tcp_server_socket_picture_services.h"

NTcpServerSocketPictureServices * NTcpServerSocketPictureServices::m_instance = NULL;

NTcpServerSocketPictureServices & NTcpServerSocketPictureServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketPictureServices();
    return *m_instance;
}

void NTcpServerSocketPictureServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketPictureServices::NTcpServerSocketPictureServices()
{
}

NTcpServerSocketPictureServices::~NTcpServerSocketPictureServices()
{
}

NResponse & NTcpServerSocketPictureServices::resize(const NClientSession & session,
                                                          NResponse & response)
{
    QString fileHash = session.resource();
    QFileInfo fileInfo = NDatabase::instance().file(fileHash);
    QSize size(0, 0);
    if (session.url().hasQueryItem("width"))
        size.setWidth(session.url().queryItemValue("width").toInt());
    if (session.url().hasQueryItem("height"))
        size.setHeight(session.url().queryItemValue("height").toInt());

    NImage image(fileInfo);
    response.setData(image.resize(size));
    response.httpHeader().setContentType(image.mimeType());
    response.removeDefaultCharset();
    return response;
}

NResponse & NTcpServerSocketPictureServices::getThumb(const NClientSession & session,
                                                         NResponse & response)
{
    QString fileHash = session.resource();
    QFileInfo fileInfo = NDatabase::instance().file(fileHash);

    NImage image(fileInfo);
    response.setData(image.getThumb());
    response.httpHeader().setContentType(image.mimeType());
    response.removeDefaultCharset();
    response.add10yExpiresHttpHeader();
    QString lastModified = NDate_n::toHTMLDateTime(fileInfo.lastModified());
    response.httpHeader().setValue("Last-Modified", lastModified);
    return response;
}
