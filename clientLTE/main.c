#ifndef STRING_H
#include <string.h>
#endif
#include "../message.h"
#include "init_connection.h"
#include "random_access.h"
#include "rrc.h"
#include "user_equipment.h"

int main(int argc, char* argv[])
{
    if(argc < 2){
        printf("You need to pass port number as an argument!\n");
        return 0;
    }
    int port_number = atoi(argv[1]);
    int socket_fd, running = 1;
    struct sockaddr_in server;
    s_message message;
    ue_battery battery;
    srand(time(NULL)); 

    initialize_battery_life(&battery);

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
        if (message.message_type == random_access_request) {
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

    if(send_rrc_connection_request(socket_fd, &message, generate_ue_identity) == -1) {
        printf("Failed to send rrc connection request!\n");
    }
    else {
        printf("Successfully send rcc connection request!\n");
    }

    int rrc_setup_complete_status = receive_rrc_setup(socket_fd, &received, &message);

    if (-1 == rrc_setup_complete_status)
        printf("Error on recv() rrc_setup.\n");
    else if (1 == rrc_setup_complete_status)
        printf("Response not OK!\n");
    else {
        printf("Response type OK!\n");
        printf("Sending rrc setup complete message...\n");
    }

    if(send_rrc_setup_complete(socket_fd, &message) == -1) {
        printf("Failed to send rrc setup!\n");
    }
    else {
        printf("Successfully send rcc setup!\n");
    }

    while (running) {
        if(update_battery(&battery) == 2)
            printf("%i", battery.power_percentage);

    }

    return 0;
}