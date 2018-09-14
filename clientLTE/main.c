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
#include <signal.h>
#include <stdbool.h>
#ifndef SHUTDOWN_H
#include "shutdown.h"
#endif

volatile bool running = true;

void signal_handler(int signum) {
    running = false;
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        add_logf(client_log_filename, LOG_ERROR, "You need to pass port number as an argument!");
        return 0;
    }
    int port_number = atoi(argv[1]);
    int socket_fd;
    struct sockaddr_in server;
    s_message message;
    ue_battery battery;
    s_cells cells;
    srand(time(NULL)); 

    struct sigaction s_signal;
    s_signal.sa_handler = signal_handler;
    sigemptyset(&s_signal.sa_mask);
    s_signal.sa_flags = 0;

    initialize_battery_life(&battery);
    initialize_cells(&cells);

    //init_connection returns 0 on error, else function returns 1
    if (init_connection(&socket_fd, &server, port_number)) {
        add_logf(client_log_filename, LOG_SUCCESS, "Connected!");
    } 
    else {
        add_logf(client_log_filename, LOG_ERROR, "Failed to connect!");
        return 0;
    }
    //returns -1 on error, else 0
    if (send_prach_preamble(socket_fd, &message, generate_ra_rnti) == -1) {
        add_logf(client_log_filename, LOG_ERROR, "Failed to send preamble!");
        return 0;
    } 
    else {
        add_logf(client_log_filename, LOG_SUCCESS, "Message sent!");
        
        if (message.message_type == random_access_request) {
            add_logf(client_log_filename, LOG_INFO, "Message type: RA_RNTI.");
        }
        else {
            add_logf(client_log_filename, LOG_WARNING, "Message type: NOT RA_RNTI.");
        }
        add_logf(client_log_filename, LOG_INFO, "RA_RNTI VALUE: %d", message.message_value.message_preamble.ra_rnti);
    }

    s_message received;

    int prach_response_func_status = receive_prach_response(socket_fd, &received, &message);

    if(-1 == prach_response_func_status) {
        add_logf(client_log_filename, LOG_ERROR, "Error on recv()!");
        return 0;
    }
    else if(1 == prach_response_func_status) {
        add_logf(client_log_filename, LOG_ERROR, "Response not OK!");
        return 0;
    }
    else {
        add_logf(client_log_filename, LOG_SUCCESS, "Response type OK.");
        add_logf(client_log_filename, LOG_SUCCESS, "RACH Success!");
    }

    if(send_rrc_connection_request(socket_fd, &message, generate_ue_identity) == -1) {
        add_logf(client_log_filename, LOG_ERROR, "Failed to send RRC connection request!");
        return 0;
    }
    else {
        add_logf(client_log_filename, LOG_SUCCESS, "Successfully sent RRC connection request.");
    }

    receive_rrc_setup(socket_fd, &received, &message);
    sigaction(SIGINT, &s_signal, NULL);

    // While running and have not received eNodeB shutdown message
    while (running && !check_for_shutdown(socket_fd, &received)) {
        update_battery(socket_fd, &message, &battery);
<<<<<<< HEAD
        set_current_signal_event(&cells);
        printf("Battery power: %i\n", battery.power_percentage);

        if (receive_ping(socket_fd, &message) == 0) {
=======
        add_logf(client_log_filename, LOG_INFO, "Battery power: %i", battery.power_percentage);
        
        if (receive_ping(socket_fd, &received) == 0)
>>>>>>> a15b75ba369432c07a11f1c26631459c692f71d0
            if (send_pong(socket_fd, &message) == -1)
                add_logf(client_log_filename, LOG_ERROR, "Failed to response to server ping!");   
            else {
                decrease_after_ping(socket_fd, &message, &battery);
                add_logf(client_log_filename, LOG_SUCCESS, "Successfully handled server ping!");
            }
<<<<<<< HEAD
        }
=======

        if (receive_measurement_control_request(socket_fd, &received))
            send_measurement_report(socket_fd, &message);
>>>>>>> a15b75ba369432c07a11f1c26631459c692f71d0

        sleep(1);
    }

    // Check if eNodeB is still on before trying to send UE off signal
    if (message.message_type != enb_off)
    {
        if(-1 == send_ue_off_signal(socket_fd, &message))
            add_logf(client_log_filename, LOG_ERROR, "Client failed to send ue_off notification!");
        else
            add_logf(client_log_filename, LOG_SUCCESS, "Client successfully disconnected from server!");
    }
    return 0;
}