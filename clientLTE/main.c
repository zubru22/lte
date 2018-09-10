#ifndef STRING_H
#include <string.h>
#endif
#ifndef MESSAGE_H
#include "../message.h"
#endif
#include "init_connection.h"
#include "random_access.h"
#include "rrc.h"
#include "user_equipment.h"
#include "../logs/logs.h"
#ifndef HANDLE_MESSAGES
#include "handle_messages.h"
#endif

int main(int argc, char* argv[])
{
    if(argc < 2){
        add_log(client_log_filename, LOG_ERROR, "You need to pass port number as an argument!");
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
        add_log(client_log_filename, LOG_SUCCESS, "Connected!");
    } 
    else {
        add_log(client_log_filename, LOG_ERROR, "Failed to connect!");
        return 0;
    }
    //returns -1 on error, else 0
    if (send_prach_preamble(socket_fd, &message, generate_ra_rnti) == -1) {
        add_log(client_log_filename, LOG_ERROR, "Failed to send preamble!");
        return 0;
    } 
    else {
        add_log(client_log_filename, LOG_SUCCESS, "Message sent!");
        //printf("Message content:\n");
        
        if (message.message_type == random_access_request) {
            add_log(client_log_filename, LOG_SUCCESS, "type: RA_RNTI.");
        }
        else {
            add_log(client_log_filename, LOG_WARNING, "type: NOT RA_RNTI.");
        }
        printf("RA_RNTI VALUE: %d\n", message.message_value.message_preamble.ra_rnti);
    }

    s_message received;

    int prach_response_func_status = receive_prach_response(socket_fd, &received, &message);

    if(-1 == prach_response_func_status) {
        add_log(client_log_filename, LOG_ERROR, "Error on recv()!");
        return 0;
    }
    else if(1 == prach_response_func_status) {
        add_log(client_log_filename, LOG_ERROR, "Response not OK!");
        return 0;
    }
    else {
        add_log(client_log_filename, LOG_SUCCESS, "Response type OK.");
        add_log(client_log_filename, LOG_SUCCESS, "RACH Success!");
    }

    if(send_rrc_connection_request(socket_fd, &message, generate_ue_identity) == -1) {
        add_log(client_log_filename, LOG_ERROR, "Failed to send RRC connection request!");
        return 0;
    }
    else {
        add_log(client_log_filename, LOG_SUCCESS, "Successfully sent RRC connection request.");
    }

    receive_rrc_setup(socket_fd, &received, &message);

    while (running) {
        update_battery(socket_fd, &message, &battery);
        printf("Battery power: %i\n", battery.power_percentage);
        if (receive_ping(socket_fd, &message) == 0)
            if (send_pong(socket_fd, &message) == -1)
                add_log(client_log_filename, LOG_ERROR, "Failed to response to server ping!");   
            else
                add_log(client_log_filename, LOG_SUCCESS, "Successfully handled server ping!");
        sleep(1);
    }

    return 0;
}