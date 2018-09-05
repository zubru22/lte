    #include <stdio.h>
    #include <string.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/types.h>
    
    int init_connection(int* socket_fd, struct sockaddr_in* server, int port_number);