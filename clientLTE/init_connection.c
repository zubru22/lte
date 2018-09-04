    #include "init_connection.h"
    
    int init_connection(int* socket_fd, struct sockaddr_in* server) {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(socket_fd == -1) {
            printf("Failed to create socket!");
            return 0;
        }

        server->sin_family = AF_INET;
        server->sin_port = htons(22000);
        inet_pton(AF_INET,"127.0.0.1",&(server->sin_addr));

        if(connect(socket_fd, (struct sockaddr *)server, sizeof(*server))< 0) {
            printf("Failed to connect to the server!\n");
            close(socket_fd);
            return 0;
        }
        else {
            printf("Connected to the server!\n");
        }
        return 1;
    }