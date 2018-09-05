#ifndef INIT_CONNECTION_H
#define INIT_CONNECTION_H
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include "random_access.h"

    int init_connection(int* socket_fd, struct sockaddr_in* server);
#endif