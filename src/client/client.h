#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <pthread.h>

#include "flow_ctrl.h"

typedef struct client {
    flow_ctrl_t flow_ctrl;
} client_t;

int client_start(client_t * client, const char * address, int port);

void client_free(client_t * client);

int client_is_running(const client_t * client);

#endif /* __CLIENT_H__ */
