#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "tcp_socket.h"

#define TCP_SERVER_BACKLOG SOMAXCONN
#define TCP_SERVER_OPT_SO_REUSEADDR_VAL 65535

static int __tcp_socket_address_to_in_addr(const char * address, in_addr_t * in_addr) {
    int ret;
    struct addrinfo * addr_list = NULL;
    struct addrinfo * addr = NULL;
    struct sockaddr_in * sockaddr = NULL;

    if (!address || !in_addr) {
        errno = EINVAL;
        return -1;
    }

    if (getaddrinfo(address, NULL, 0, &addr_list)) {
        perror("tcp_socket (getaddrinfo)");
        return -1;
    }

    for (addr = addr_list; addr; addr = addr->ai_next) {
        if (addr->ai_addr->sa_family == AF_INET) {
            sockaddr = (struct sockaddr_in *) addr->ai_addr;
            break;
        }
    }

    if (sockaddr) {
        memcpy((void *) in_addr, (void *) &(sockaddr->sin_addr.s_addr), sizeof(in_addr_t));
        ret = 0;
    } else {
        errno = ENOENT;
        perror("tcp_socket (getaddrinfo)");
        ret = -1;
    }

    freeaddrinfo(addr_list);

    return ret;
}

static ssize_t __tcp_socket_read(const tcp_socket_t * sock, void * buf, size_t buf_len, int flags) {
    if (!sock || !buf) {
        errno = EINVAL;
        perror("tcp_socket");
        return -1;
    }

    return recv(sock->fd, buf, buf_len, flags);
}

static ssize_t __tcp_socket_write(const tcp_socket_t * sock, const void * buf, size_t buf_len, int flags) {
    if (!sock || !buf) {
        errno = EINVAL;
        perror("tcp_socket");
        return -1;
    }

    return send(sock->fd, buf, buf_len, flags);
}

static int __tcp_socket_client_connect(tcp_socket_t * client, int reconnect) {
    if (!client) {
        errno = EINVAL;
        return -1;
    }

    if (!client->fd && !reconnect) {
        errno = EBADFD;
        return -1;
    }

    if (reconnect) {
        tcp_socket_free(client);
        if ((client->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            return -1;
        }
    }

    return connect(client->fd, (struct sockaddr *) &(client->addr), sizeof(struct sockaddr));
}

int tcp_socket_init(tcp_socket_t * sock, const char * address, int port) {
    in_addr_t in_addr = INADDR_ANY;

    if (!sock || port < 0 || port > 65535) {
        errno = EINVAL;
        perror("tcp_socket");
        return -1;
    }

    memset((void *) sock, 0, sizeof(tcp_socket_t));

    if ((sock->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("tcp_socket (socket)");
        return -1;
    }

    sock->addr.sin_family = AF_INET;
    sock->addr.sin_port = htons(port);
    sock->addr.sin_addr.s_addr = in_addr;

    if (!address) {
        return 0;
    }

    in_addr = inet_addr(address);

    if (in_addr != INADDR_NONE) {
        sock->addr.sin_addr.s_addr = in_addr;
        return 0;
    }

    if (__tcp_socket_address_to_in_addr(address, &in_addr)) {
        return -1;
    }

    return 0;
}

void tcp_socket_free(tcp_socket_t * sock) {
    if (!sock) {
        return;
    }

    if (sock->fd) {
        close(sock->fd);
        sock->fd = 0;
    }
}

ssize_t tcp_socket_read(const tcp_socket_t * sock, void * buf, size_t buf_len) {
    return  __tcp_socket_read(sock, buf, buf_len, 0);
}

ssize_t tcp_socket_read_nonblock(const tcp_socket_t * sock, void * buf, size_t buf_len) {
    return  __tcp_socket_read(sock, buf, buf_len, MSG_DONTWAIT);
}

ssize_t tcp_socket_write(const tcp_socket_t * sock, const void * buf, size_t buf_len) {
    return __tcp_socket_write(sock, buf, buf_len, MSG_NOSIGNAL);
}

int tcp_socket_server_start(tcp_socket_t * server) {
    int optval = TCP_SERVER_OPT_SO_REUSEADDR_VAL;

    if (!server) {
        errno = EINVAL;
        return -1;
    }

    if (!server->fd) {
        errno = EBADFD;
        return -1;
    }

    if ((setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) < 0) {
        perror("tcp_socket (setsockopt)");
        return -1;
    }

    if (bind(server->fd, (struct sockaddr *) &(server->addr), sizeof(struct sockaddr_in)) < 0) {
        perror("tcp_socket (bind)");
        return -1;
    }

    if (listen(server->fd , TCP_SERVER_BACKLOG) < 0) {
        perror("tcp_socket (listen)");
        return -1;
    }

    return 0;
}

int tcp_socket_server_wait_new_conn(tcp_socket_t * server, tcp_socket_t * conn) {
    int addrlen = sizeof(struct sockaddr_in);

    if (!server || !conn) {
        errno = EINVAL;
        return -1;
    }

    memset((void *) conn, 0, sizeof(tcp_socket_t));

    conn->fd = accept(server->fd, (struct sockaddr *) &(conn->addr), (socklen_t*) &addrlen);

    if (conn->fd < 0) {
        perror("tcp_socket (accept)");
        return -1;
    }

    return 0;
}

int tcp_socket_client_connect(tcp_socket_t * client) {
    return __tcp_socket_client_connect(client, 0);
}

int tcp_socket_client_reconnect(tcp_socket_t * client) {
    return __tcp_socket_client_connect(client, 1);
}