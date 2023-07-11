#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "flow_ctrl.h"

static const struct timespec flow_ctrl_task_period = {
    .tv_sec = 0,
    .tv_nsec = 1000000 /* 1ms */
};

static void * __flow_ctrl_sock_task(void * args) {
    char c;
    ssize_t size;
    flow_ctrl_t * flow_ctrl;

    flow_ctrl = (flow_ctrl_t *) args;
    flow_ctrl->is_running = 1;

    while (flow_ctrl->is_running) {
        if (!(flow_ctrl->is_connected)) {
            if (!(flow_ctrl->connect_cb) || flow_ctrl->connect_cb(flow_ctrl) <= 0) {
                nanosleep(&flow_ctrl_task_period, NULL);
                continue;
            }
            printf("\r\n[connected]\r\n");
            pthread_mutex_lock(&(flow_ctrl->mutex));
            flow_ctrl->is_connected = 1;
            pthread_mutex_unlock(&(flow_ctrl->mutex));
        }

        pthread_mutex_lock(&(flow_ctrl->mutex));
        size = tcp_socket_read_nonblock(&(flow_ctrl->sock), &c, sizeof(c));
        pthread_mutex_unlock(&(flow_ctrl->mutex));

        if (size > 0) {
            printf("%c", c);
            fflush(stdout);
        } else if (size == 0) {
            printf("\r\n[disconnected]\r\n");
            pthread_mutex_lock(&(flow_ctrl->mutex));
            flow_ctrl->is_connected = 0;
            pthread_mutex_unlock(&(flow_ctrl->mutex));
        }

        nanosleep(&flow_ctrl_task_period, NULL);
    }

    return NULL;
}

static void * __flow_ctrl_term_task(void * args) {
    char c;
    ssize_t size;
    flow_ctrl_t * flow_ctrl;

    flow_ctrl = (flow_ctrl_t *) args;
    flow_ctrl->is_running = 1;

    while (flow_ctrl->is_running) {
        c = getchar();
        size = 0;

        if (flow_ctrl->is_connected) {
            pthread_mutex_lock(&(flow_ctrl->mutex));
            size = tcp_socket_write(&(flow_ctrl->sock), &c, 1);
            pthread_mutex_unlock(&(flow_ctrl->mutex));
        }

        if (size < 0) {
            printf("\r\n[disconnected]\r\n");
            pthread_mutex_lock(&(flow_ctrl->mutex));
            flow_ctrl->is_connected = 0;
            pthread_mutex_unlock(&(flow_ctrl->mutex));
        }

        nanosleep(&flow_ctrl_task_period, NULL);
    }

    return NULL;
}

int flow_ctrl_start(flow_ctrl_t * flow_ctrl, flow_ctrl_connect_cb connect_cb, void * user_data) {
    if (!flow_ctrl || !connect_cb) {
        errno = EINVAL;
        return -1;
    }

    memset((void *) flow_ctrl, 0, sizeof(flow_ctrl_t));

    flow_ctrl->user_data = user_data;
    flow_ctrl->connect_cb = connect_cb;

    if (pthread_mutex_init(&(flow_ctrl->mutex), NULL) != 0) {
        perror("flow_ctrl (pthread_mutex_init)");
        flow_ctrl_free(flow_ctrl);
        return -1;
    }

    if (pthread_create(&(flow_ctrl->term_tid), NULL, &__flow_ctrl_term_task, flow_ctrl) < 0) {
        perror("flow_ctrl (pthread_create)");
        flow_ctrl_free(flow_ctrl);
        return -1;
    }

    if (pthread_create(&(flow_ctrl->sock_tid), NULL, &__flow_ctrl_sock_task, flow_ctrl) < 0) {
        perror("flow_ctrl (pthread_create)");
        flow_ctrl_free(flow_ctrl);
        return -1;
    }

    return 0;
}

void flow_ctrl_free(flow_ctrl_t * flow_ctrl) {
    if (!flow_ctrl) {
        return;
    }

    flow_ctrl->is_running = 0;

    if (flow_ctrl->term_tid) {
        pthread_cancel(flow_ctrl->term_tid);
        pthread_join(flow_ctrl->term_tid, NULL);
    }

    if (flow_ctrl->sock_tid) {
        pthread_cancel(flow_ctrl->sock_tid);
        pthread_join(flow_ctrl->sock_tid, NULL);
    }

    tcp_socket_free(&(flow_ctrl->sock));

    pthread_mutex_destroy(&(flow_ctrl->mutex));
}
