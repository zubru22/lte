#include "init_connection.h"

int main(int argc, char* argv[]) {
    int* socket_fd;
    struct sockaddr_in server;

    //init_connection returns 0 on error, else function returns 1
    if(init_connection(socket_fd, &server)) {
        printf("Connected!\n");
    }
    else {
        printf("Failed to connect!");
    }

}