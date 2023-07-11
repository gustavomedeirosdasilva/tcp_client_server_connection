#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "server.h"

static const struct timespec task_period = {
    .tv_sec = 0,
    .tv_nsec = 1000000 /* 1ms */
};

static void * __server_sock_task(void * args) {
    char c;
    ssize_t size;
    server_t * server;

    server = (server_t *) args;
    server->is_running = 1;

    while (server->is_running) {
        if (!(server->is_connected)) {
            if (tcp_socket_server_wait_new_conn(&(server->tcp_server), &(server->sock))) {
                nanosleep(&task_period, NULL);
                continue;
            }
            server->is_connected = 1;
        }

        pthread_mutex_lock(&(server->mutex));
        size = tcp_socket_read_nonblock(&(server->sock), &c, sizeof(c));
        pthread_mutex_unlock(&(server->mutex));

        if (size > 0) {
            printf("%c", c);
            fflush(stdout);
        } else if (size == 0) {
            server->is_connected = 0;
        }

        nanosleep(&task_period, NULL);
    }

    return NULL;
}

static void * __server_term_task(void * args) {
    char c;
    server_t * server;

    server = (server_t *) args;
    server->is_running = 1;

    while (server->is_running) {
        c = getchar();

        pthread_mutex_lock(&(server->mutex));
        if (server->is_connected) {
            tcp_socket_write(&(server->sock), &c, 1);
        }
        pthread_mutex_unlock(&(server->mutex));
    }

    return NULL;
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

    if (pthread_mutex_init(&(server->mutex), NULL) != 0) {
        server_free(server);
        return -1;
    }

    if (pthread_create(&(server->term_tid), NULL, &__server_term_task, server) < 0) {
        server_free(server);
        return -1;
    }

    if (pthread_create(&(server->sock_tid), NULL, &__server_sock_task, server) < 0) {
        server_free(server);
        return -1;
    }

    return 0;
}

void server_free(server_t * server) {
    if (!server) {
        return;
    }

    server->is_running = 0;

    if (server->term_tid) {
        pthread_cancel(server->term_tid);
        pthread_join(server->term_tid, NULL);
    }

    if (server->sock_tid) {
        pthread_cancel(server->sock_tid);
        pthread_join(server->sock_tid, NULL);
    }

    tcp_socket_free(&(server->tcp_server));
    tcp_socket_free(&(server->sock));

    pthread_mutex_destroy(&(server->mutex));
}

int server_is_running(const server_t * server) {
    if (!server) {
        errno = EINVAL;
        return -1;
    }

    return server->is_running;
}
