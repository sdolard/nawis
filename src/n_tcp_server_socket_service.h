#ifndef N_TCP_SERVER_SOCKET_SERVICE_H
#define N_TCP_SERVER_SOCKET_SERVICE_H

#include <QScriptValue>

#include "n_response.h"
#include "n_services.h"
#include "n_client_session.h"

#define RSP_SUCCESS                    "success"
#define RSP_MSG                        "message"
#define RSP_MSG_LOADED                 "Data loaded"
#define RSP_MSG_NO_RESULTS             "No results"
#define RSP_MSG_N_DELETED              "%1 deleted"
#define RSP_MSG_N_UPDATED              "Data %1 updated"
#define RSP_MSG_INVALID_JSON           "invalid JSON"
#define RSP_MSG_INVALID_INDEX          "invalid index"
#define RSP_MSG_INVALID_INDEX_PROPERTY "invalid index property"
#define RSP_MSG_INVALID_USER           "invalid user"
#define RSP_MSG_ERROR_OCCURRED         "An error occured"
#define RSP_COUNT                      "totalcount"
#define RSP_DATA                       "data"
#define RSP_RESULT                     "result"


// Base class for all web services
class NTcpServerSocketService
{
public:
    NTcpServerSocketService();
    ~NTcpServerSocketService();

    static NResponse & getHelp(NService_n::NService* services, NResponse & response);
    static NResponse & getFullHelp(NService_n::NService* rootServices, NResponse & response);
    static void setJsonRootReponse(QScriptValue & svRoot, int totalCount, bool succeed);
};

#endif // N_TCP_SERVER_SOCKET_SERVICE_H
