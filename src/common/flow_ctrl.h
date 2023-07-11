#ifndef __FLOW_CTRL_H__
#define __FLOW_CTRL_H__

#include <pthread.h>

#include "tcp_socket.h"

typedef struct flow_ctrl flow_ctrl_t;
typedef int (* flow_ctrl_connect_cb)(flow_ctrl_t * flow_ctrl);

struct flow_ctrl {
    tcp_socket_t sock;
    pthread_t sock_tid;
    pthread_t term_tid;
    pthread_mutex_t mutex;
    int is_connected;
    int is_running;
    void * user_data;
    flow_ctrl_connect_cb connect_cb;
};

int flow_ctrl_start(flow_ctrl_t * flow_ctrl, flow_ctrl_connect_cb connect_cb, void * user_data);

void flow_ctrl_free(flow_ctrl_t * flow_ctrl);

#endif /* __FLOW_CTRL_H__ */
