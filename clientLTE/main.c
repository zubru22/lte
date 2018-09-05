#include "../message.h"
#include "init_connection.h"
#include "random_access.h"

int main(int argc, char* argv[])
{
    if(argc < 2){
        printf("You need to pass port number as an argument!\n");
        return 0;
    }
    int port_number = atoi(argv[1]);
    int socket_fd;
    struct sockaddr_in server;
    s_message message;
    srand(time(NULL));
    printf("%i", argc);
    

    //init_connection returns 0 on error, else function returns 1
    if (init_connection(&socket_fd, &server, port_number)) {
        printf("Connected!\n");
    } else {
        printf("Failed to connect!\n");
    }
    //returns -1 on error, else 0
    if (send_prach_preamble(socket_fd, &message, generate_ra_rnti) == -1) {
        printf("Failed to send!\n");
    } else {
        printf("Message sent!\n");
        printf("Message content:\n");
        if (message.message_type == RA_RNTI) {
            printf("type: RA_RNTI\n");
        } else {
            printf("type: NOT RA_RNTI\n");
        }
        printf("RA_RNTI VALUE: %d\n", message.message_value.message_preamble.ra_rnti);
    }

    s_message received;

    int prach_response_func_status = receive_prach_response(socket_fd, &received, &message);

    if(-1 == prach_response_func_status) 
        printf("Error on recv()\n");
    else if(1 == prach_response_func_status)
        printf("Response not OK\n");
    else {
        printf("Response type OK\n");
        printf("RACH SUCCESS\n");
    }
    return 0;
}