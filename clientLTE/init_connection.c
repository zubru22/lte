    #include "init_connection.h"
    #include "../logs/logs.h"

    int init_connection(int* socket_fd, struct sockaddr_in* server, int port_number) {
        *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(*socket_fd == -1) {
            add_log(client_log_filename, LOG_ERROR, "Failed to create socket!");
            return 0;
        }

        server->sin_family = AF_INET;
        server->sin_port = htons(port_number);
        inet_pton(AF_INET,"127.0.0.1",&(server->sin_addr));

        if(connect(*socket_fd, (struct sockaddr *)server, sizeof(*server)) < 0) {
            add_log(client_log_filename, LOG_ERROR, "Failed to connect to the server!");
            close(*socket_fd);
            return 0;
        }
        else {
            add_log(client_log_filename, LOG_SUCCESS, "Connected to the server!");
        }
        return 1;
    }