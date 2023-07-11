#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <pthread.h>

#include "tcp_socket.h"

typedef struct client {
    tcp_socket_t sock;
    pthread_t sock_tid;
    pthread_t term_tid;
    pthread_mutex_t mutex;
    int is_connected;
    int is_running;
} client_t;

int client_start(client_t * client, const char * address, int port);
void client_free(client_t * client);
int client_is_running(const client_t * client);

#endif /* __CLIENT_H__ */
