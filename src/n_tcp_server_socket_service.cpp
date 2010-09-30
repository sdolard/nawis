#include <QString>

#include "n_tcp_server_auth_session.h"
#include "n_version.h"

#include "n_tcp_server_socket_service.h"

NTcpServerSocketService::NTcpServerSocketService()
{
}

NTcpServerSocketService::~NTcpServerSocketService()
{
}


NResponse & NTcpServerSocketService::getHelp(NService_n::NService* services, NResponse & response)
{
    Q_ASSERT(services);

    QString htmlHelpTemplate = "<html><head><title>%1</title>"\
                               "</head><body>%2</body></html>";
    htmlHelpTemplate = htmlHelpTemplate.arg(QString("nawis server api %1 %2").
                                            arg(getNawisVersion()).
                                            arg(getNawisBuild()));
    QString htmlCommandTemplate = "<h2>Command: %1</h2>";
    QString servicesHelp;
    int i = 0;
    while (true)
    {
        if (services[i].id == SVC_NONE)
            break;
        servicesHelp += htmlCommandTemplate.arg(services[i].fullService);
        servicesHelp += QString("<p>%1<br><b>Appears in</b>: %2<br>"\
                                "<b>Authentication required</b>: %3<br>"\
                                "<b>Required level</b>: \"%4\"<br>"\
                                "<b>HTTP method</b>: %5<br>"\
                                "<b>URL params</b>:<br> %6<br>"\
                                "<b>Post data</b>: %7<br>"\
                                "<b>Return</b>: %8</p>").
                arg(services[i].comment).
                arg(services[i].history).
                arg(services[i].authRequired ? "yes" : "no").
                arg(NTcpServerAuthSession::levelToString(services[i].requiredLevel)).
                arg(services[i].httpMethod).
                arg(services[i].params.isEmpty() ? "none": services[i].params).
                arg(services[i].content.isEmpty() ? "none": services[i].content).
                arg(services[i].returns);

        servicesHelp += "<hr />";
        ++i;
    }
    response.setData(htmlHelpTemplate.arg(servicesHelp).toUtf8());
    response.httpHeader().setContentType("text/html");
    return response;
}

NResponse & NTcpServerSocketService::getFullHelp(NService_n::NService* rootServices, NResponse & response)
{
    int size = 0;
    NService_n::NService services[999];
    NService_n::getAllServices(rootServices, services, &size);
    return getHelp(services, response);
}

