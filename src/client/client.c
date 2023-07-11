#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "client.h"

static const struct timespec task_period = {
    .tv_sec = 0,
    .tv_nsec = 1000000 /* 1ms */
};

static void * __client_sock_task(void * args) {
    char c;
    ssize_t size;
    client_t * client;

    client = (client_t *) args;
    client->is_running = 1;

    while (client->is_running) {
        if (!(client->is_connected)) {
            if (tcp_socket_client_reconnect(&(client->sock)) < 0) {
                nanosleep(&task_period, NULL);
                continue;
            }

            client->is_connected = 1;
        }

        pthread_mutex_lock(&(client->mutex));
        size = tcp_socket_read_nonblock(&(client->sock), &c, sizeof(c));
        pthread_mutex_unlock(&(client->mutex));

        if (size > 0) {
            printf("%c", c);
            fflush(stdout);
        } else if (size == 0) {
            client->is_connected = 0;
        }

        nanosleep(&task_period, NULL);
    }

    return NULL;
}

static void * __client_term_task(void * args) {
    char c;
    client_t * client;

    client = (client_t *) args;
    client->is_running = 1;

    while (client->is_running) {
        c = getchar();

        pthread_mutex_lock(&(client->mutex));
        if (client->is_connected) {
            tcp_socket_write(&(client->sock), &c, 1);
        }
        pthread_mutex_unlock(&(client->mutex));
    }

    return NULL;
}

int client_start(client_t * client, const char * address, int port) {
    if (!client || !address) {
        errno = EINVAL;
        return -1;
    }

    memset((void *) client, 0, sizeof(client_t));

    if (tcp_socket_init(&(client->sock), address, port)) {
        client_free(client);
        return -1;
    }

    if (pthread_mutex_init(&(client->mutex), NULL) != 0) {
        client_free(client);
        return -1;
    }

    if (pthread_create(&(client->term_tid), NULL, &__client_term_task, client) < 0) {
        client_free(client);
        return -1;
    }

    if (pthread_create(&(client->sock_tid), NULL, &__client_sock_task, client) < 0) {
        client_free(client);
        return -1;
    }

    return 0;
}

void client_free(client_t * client) {
    if (!client) {
        return;
    }

    client->is_running = 0;

    if (client->term_tid) {
        pthread_cancel(client->term_tid);
        pthread_join(client->term_tid, NULL);
    }

    if (client->sock_tid) {
        pthread_cancel(client->sock_tid);
        pthread_join(client->sock_tid, NULL);
    }

    tcp_socket_free(&(client->sock));

    pthread_mutex_destroy(&(client->mutex));
}

int client_is_running(const client_t * client) {
    if (!client) {
        errno = EINVAL;
        return -1;
    }

    return client->is_running;
}
