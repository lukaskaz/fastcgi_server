#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <thread>

#include "constants.hpp"
#include "connection.hpp"
#include "server.hpp"

#define FCGI_SERVER_DEFAULT_PORT    8080


const uint16_t server::port = FCGI_SERVER_DEFAULT_PORT;

server::~server()
{
	close(sock_fd);
}

server::server(responder &resp, uint16_t defaultPort):
    Log(SERV_LOG_SIMPLE, SERV_LOG_ERROR, SERV_LOG_INFO, SERV_LOG_DEBUG),
    class_name(PREP_CLASS_NAME),
    sock_fd(-1), user_responder(resp), conn_nb(0)
{
    int32_t tmp_fd = (-1);

    do {
        tmp_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(tmp_fd < 0) {
            LOG_ERROR(FUNC_ID(class_name), "Cannot create communication socket");
            break;
        }

        const int32_t reuse = 1;
        if(setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, (size_t)sizeof(reuse)) < 0) {
            LOG_ERROR(FUNC_ID(class_name), "Cannot set socket to reuse address");
            break;
        }

        if(fcntl(tmp_fd, F_SETFL, fcntl(tmp_fd, F_GETFL) | O_NONBLOCK) < 0) {
            LOG_ERROR(FUNC_ID(class_name), "Cannot enable non-blocking for socket");
            break;
        }

        struct sockaddr_in sockadd = {0};
        sockadd.sin_family = AF_INET;
        sockadd.sin_port = htons(port);
        sockadd.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(tmp_fd, (struct sockaddr *)&sockadd, (size_t)sizeof(sockadd)) < 0) {
            LOG_ERROR(FUNC_ID(class_name), "Cannot assign address/port to socket");
            break;
        }

        sock_fd = tmp_fd;
    }while(false);

    if(sock_fd == (-1) && tmp_fd >= 0) {
        close(tmp_fd);
        tmp_fd = (-1);
    }
}

void server::handleRequests(void)
{
    if(sock_fd >= 0) {
        struct epoll_event event = {0}, revent = {0};
        int32_t epoll_fd = (-1);

        event.data.fd = sock_fd;
        event.events = EPOLLIN | EPOLLET;

        epoll_fd = epoll_create1(0);
        if(epoll_fd >= 0) {
            if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == 0) {
                listen(sock_fd, FCGI_MAX_CONNS_QUEUE_SIZE);

                while(epoll_wait(epoll_fd, &revent, 1, FCGI_POLL_WAIT_INFINITE) > 0) {
                    if(revent.events & EPOLLIN) {
                        deployConnection();
                    }
                    else {
                        //error case (EPOLLHUP|EPOLLERR), terminate main socket
                        LOG_ERROR(FUNC_ID(class_name), "Socket in error state, terminating handler");
                        shutdown(sock_fd, SHUT_RDWR);
                        close(sock_fd);
                        sock_fd = (-1);
                        break;
                    }
                }
            }

            close(epoll_fd);
        }
        else {
            LOG_ERROR(FUNC_ID(class_name), "Cannot initiate monitoring for socket events");
        }
    }
}

void server::deployConnection(void)
{
    int8_t pending_conns = connectionsNumber(PEND_CONNS_GET);

    if(pending_conns < FCGI_MAX_CONNS) {
        int32_t fd = accept(sock_fd, NULL, NULL);
        if(fd >= 0) {
            pending_conns = connectionsNumber(PEND_CONNS_INCR);

            LOG_INFO(FUNC_ID(class_name), "Deploying new connection number: " +
                                    CONV_NUM_GET_STR(int8_t, dec, pending_conns));

            std::thread connThd(&server::connectionThread, this, fd);
            connThd.detach();
        }
        else {
            LOG_ERROR(FUNC_ID(class_name), "Cannot create pending connection request");
        }
    }
}

void server::connectionThread(int32_t fd)
{
    connection conn(fd, user_responder);

    LOG_INFO(FUNC_ID(class_name), "Thread ready to process request");
    while(true) {
        LOG_DEBUG(FUNC_ID(class_name), "Thread is collecting data");
        if(conn.readHeader() == 0) {
            LOG_DEBUG(FUNC_ID(class_name), "Thread received header");
            if(conn.readBody() == 0) {
                LOG_DEBUG(FUNC_ID(class_name), "Thread received body");
                if(conn.process() == 0) {
                    LOG_DEBUG(FUNC_ID(class_name), "Thread processed given " \
                            "request type, preparing to receive next request type");
                    continue;
                }
            }
        }

        break;
    }

    int8_t conns = connectionsNumber(PEND_CONNS_DECR);
    LOG_INFO(FUNC_ID(class_name), "Thread job completed, ending this thread");
    LOG_INFO(FUNC_ID(class_name), "Number of other connections that are being " \
                        "processed now: " + CONV_NUM_GET_STR(int8_t, dec, conns));
}

int8_t server::connectionsNumber(connsNbOp_t op)
{
    int8_t conns_nb_tmp = (-1);

    std::unique_lock<std::mutex> lock(conn_mtx);
    switch(op) {
        case PEND_CONNS_GET:
            break;
        case PEND_CONNS_INCR:
            conn_nb++;
            break;
        case PEND_CONNS_DECR:
            conn_nb--;
            break;
        default:
            break;
    }
    conns_nb_tmp = conn_nb;
    lock.unlock();

    return conns_nb_tmp;
}
