#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include <netdb.h>

typedef struct tcp_socket {
    int fd;
    struct sockaddr_in addr;
} tcp_socket_t;

/* Common socket calls */

int tcp_socket_init(tcp_socket_t * sock, const char * address, int port);

void tcp_socket_free(tcp_socket_t * sock);

ssize_t tcp_socket_read(const tcp_socket_t * sock, void * buf, size_t buf_len);

ssize_t tcp_socket_read_nonblock(const tcp_socket_t * sock, void * buf, size_t buf_len);

ssize_t tcp_socket_write(const tcp_socket_t * sock, const void * buf, size_t buf_len);

/* Server socket calls */

int tcp_socket_server_start(tcp_socket_t * server);

int tcp_socket_server_wait_new_conn(tcp_socket_t * server, tcp_socket_t * conn);

/* Client socket calls */

int tcp_socket_client_connect(tcp_socket_t * client);

int tcp_socket_client_reconnect(tcp_socket_t * client);

#endif /* __TCP_SOCKET_H__ */