#include <stdio.h>
#include <time.h>

#include "client.h"

int main(int argc, char ** argv) {
    int port;
    char * address;
    client_t client;
    struct timespec task_period = {
        .tv_sec = 0,
        .tv_nsec = 1000000 /* 1 ms */
    };

    if (argc < 3) {
        printf("%s <address> <port>\n", argv[0]);
        return 1;
    }

    address = argv[1];

    if (sscanf(argv[2], "%d", &port) != 1) {
        printf("Invalid port\n");
        return 1;
    }

    if (client_start(&client, address, port) < 0) {
        return 1;
    }

    printf("Hash (#) character ends the program.\n");

    while (client_is_running(&client) == 1) {
        nanosleep(&task_period, NULL);
    }

    client_free(&client);

    return 0;
}
