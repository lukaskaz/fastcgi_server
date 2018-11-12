#ifndef __SERVER_H__
#define __SERVER_H__

#include <mutex>
#include "responder.hpp"

#define SERV_LOG_SIMPLE     false
#define SERV_LOG_ERROR      true
#define SERV_LOG_INFO       true
#define SERV_LOG_DEBUG      true

class server: public Log {
public:
    const std::string class_name;

    server(responder &res, uint16_t defaultPort = port);
    ~server();

    void handleRequests(void);

private:
    enum connsNbOp_t { PEND_CONNS_GET = 0, PEND_CONNS_INCR, PEND_CONNS_DECR };
    static const uint16_t port;
    int32_t sock_fd;

    responder &user_responder;
    std::mutex conn_mtx;
    int8_t conn_nb;

    void deployConnection(void);
    void connectionThread(int32_t fd);

    int8_t connectionsNumber(connsNbOp_t op);
};

#endif
