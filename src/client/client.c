#include <string.h>
#include <errno.h>

#include "client.h"

static int __client_connect_handler(flow_ctrl_t * flow_ctrl) {
    if (!flow_ctrl) {
        return -1;
    }

    if (tcp_socket_client_reconnect(&(flow_ctrl->sock)) < 0) {
        return 0;
    }

    return 1;
}

int client_start(client_t * client, const char * address, int port) {
    if (!client) {
        errno = EINVAL;
        return -1;
    }

    memset((void *) client, 0, sizeof(client_t));

    if (flow_ctrl_start(&(client->flow_ctrl), __client_connect_handler, NULL) < 0) {
        return -1;
    }

    if (tcp_socket_init(&(client->flow_ctrl.sock), address, port)) {
        client_free(client);
        return -1;
    }

    return 0;
}

void client_free(client_t * client) {
    if (!client) {
        return;
    }

    flow_ctrl_free(&(client->flow_ctrl));
}

int client_is_running(const client_t * client) {
    if (!client) {
        errno = EINVAL;
        return -1;
    }

    return client->flow_ctrl.is_running;
}
