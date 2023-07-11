#include <string.h>
#include <errno.h>

#include "server.h"

static int __server_connect_handler(flow_ctrl_t * flow_ctrl) {
    server_t * server;

    if (!flow_ctrl) {
        return -1;
    }

    server = (server_t *) flow_ctrl->user_data;

    if (tcp_socket_server_wait_new_conn(&(server->tcp_server), &(flow_ctrl->sock))) {
        return 0;
    }

    return 1;
}

int server_start(server_t * server, int port) {
    if (!server) {
        errno = EINVAL;
        return -1;
    }

    memset((void *) server, 0, sizeof(server_t));

    if (tcp_socket_init(&(server->tcp_server), NULL, port)) {
        server_free(server);
        return -1;
    }

    if (tcp_socket_server_start(&(server->tcp_server))) {
        server_free(server);
        return -1;
    }

    if (flow_ctrl_start(&(server->flow_ctrl), __server_connect_handler, (void *) server) < 0) {
        return -1;
    }

    return 0;
}

void server_free(server_t * server) {
    if (!server) {
        return;
    }

    tcp_socket_free(&(server->tcp_server));

    flow_ctrl_free(&(server->flow_ctrl));
}

int server_is_running(const server_t * server) {
    if (!server) {
        errno = EINVAL;
        return -1;
    }

    return server->flow_ctrl.is_running;
}
