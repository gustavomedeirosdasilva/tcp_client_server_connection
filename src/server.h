#ifndef __SERVER_H__
#define __SERVER_H__

#include <pthread.h>

#include "tcp_socket.h"

typedef struct server {
    tcp_socket_t sock;
    pthread_t sock_tid;
    pthread_t term_tid;
    pthread_mutex_t mutex;
    tcp_socket_t tcp_server;
    int is_connected;
    int is_running;
} server_t;

int server_start(server_t * server, int port);
void server_free(server_t * server);
int server_is_running(const server_t * server);

#endif /* __SERVER_H__ */
