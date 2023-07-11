#ifndef __SERVER_H__
#define __SERVER_H__

#include <pthread.h>

#include "flow_ctrl.h"
#include "tcp_socket.h"

typedef struct server {
    flow_ctrl_t flow_ctrl;
    tcp_socket_t tcp_server;
} server_t;

int server_start(server_t * server, int port);

void server_free(server_t * server);

int server_is_running(const server_t * server);

#endif /* __SERVER_H__ */
