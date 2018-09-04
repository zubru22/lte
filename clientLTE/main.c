#include "init_connection.h"
#include "random_access.h"
#include "../message.h"

int main(int argc, char* argv[]) {
    int socket_fd;
    struct sockaddr_in server;
    s_message message;

    srand(time(NULL));

    //init_connection returns 0 on error, else function returns 1
    if(init_connection(&socket_fd, &server)) {
        printf("Connected!\n");
    }
    else {
        printf("Failed to connect!");
    }
    //returns -1 on error, else 0
    if(send_prach_preamble(socket_fd, &message, generate_ra_rnti) == -1) {
        printf("Failed to send!");
    }


}