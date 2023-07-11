#include <stdio.h>
#include <time.h>

#include "server.h"

int main(int argc, char ** argv) {
    int port;
    server_t server;
    struct timespec task_period = {
        .tv_sec = 0,
        .tv_nsec = 1000000
    };

    if (argc < 2) {
        printf("%s <port>\n", argv[0]);
        return 1;
    }

    if (sscanf(argv[1], "%d", &port) != 1) {
        printf("Invalid port\n");
        return 1;
    }

    if (server_start(&server, port) < 0) {
        return 1;
    }

    while (server_is_running(&server) == 1) {
        nanosleep(&task_period, NULL);
    }

    server_free(&server);

    return 0;
}
