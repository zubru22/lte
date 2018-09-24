    #include "init_connection.h"
    #include "../logs/logs.h"

    static char client_log_filename[] = "../logs/client.log";

    int init_connection(int* socket_fd, struct sockaddr_in* server, int port_number, const char* ip_addr) {
        *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(*socket_fd == -1) {
            add_logf(client_log_filename, LOG_ERROR, "Failed to create socket!");
            return 0;
        }
        printf ("port = %d\n", port_number);
        printf ("ip_address = %s\n", ip_addr);
        server->sin_family = AF_INET;
        server->sin_port = htons(port_number);
        inet_pton(AF_INET,ip_addr,&(server->sin_addr));

        if(connect(*socket_fd, (struct sockaddr *)server, sizeof(*server)) < 0) {
            add_logf(client_log_filename, LOG_ERROR, "Failed to connect to the server!");
            close(*socket_fd);
            return 0;
        }
        else {
            add_logf(client_log_filename, LOG_SUCCESS, "Connected to another eNodeB!");
        }
        return 1;
    }
