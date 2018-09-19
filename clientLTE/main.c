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
#ifndef LOGS_H
#include "../logs/logs.h"
#endif
#ifndef HANDLE_MESSAGES
#include "handle_messages.h"
#endif
#include <signal.h>
#include <stdbool.h>
#ifndef SHUTDOWN_H
#include "shutdown.h"
#endif
#ifndef PTHREAD_H
#include <pthread.h>
#endif

volatile bool running = true;
int socket_fd;
s_message message;
ue_battery battery;

void signal_handler(int signum) {
    running = false;
}

void* ping_thread() {
    while(running) {
        if (receive_ping(socket_fd, &message) == 0) {
            if (send_pong(socket_fd, &message) == -1)
                add_logf(client_log_filename, LOG_ERROR, "Failed to response to server ping!");
            else {
                decrease_after_ping(socket_fd, &message, &battery);
                add_logf(client_log_filename, LOG_SUCCESS, "Successfully handled server ping!");
            }
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        add_logf(client_log_filename, LOG_ERROR, "You need to pass port number as an argument!");
        return 0;
    }
    int port_number = atoi(argv[1]);
    int socket_fd;
    int download_data_return_value;
    struct sockaddr_in server;
    s_message message;
    s_message* message_pointer = &message;
    ue_battery battery;
    s_cells cells;
    FILE* file_to_recv;
    pthread_t ping_thread_id;

    srand(time(NULL)); 

    struct sigaction s_signal;
    s_signal.sa_handler = signal_handler;
    sigemptyset(&s_signal.sa_mask);
    s_signal.sa_flags = 0;


    // INITIALIZATION BLOCK /////////////////////////////////////

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

    /////////////////////////////////////////////////////////////

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
        recv(socket_fd, (s_message*)message_pointer, sizeof(message), MSG_DONTWAIT);
        switch (message.message_type) {
            case ping:                
                if (receive_ping(socket_fd, &message) == 0) {
                    if (send_pong(socket_fd, &message) == -1)
                        add_logf(client_log_filename, LOG_ERROR, "Failed to response to server ping!");   
                    else {
                        decrease_after_ping(socket_fd, &message, &battery);
                        add_logf(client_log_filename, LOG_SUCCESS, "Successfully handled server ping!");
                    }
                }
                break;
            case data_start:
                file_to_recv = fopen("received","ab+");
                printf("\n\n----------------------\nStarted downloading data!\n----------------------\n\n");    
                break;
            case data:
                download_data(socket_fd, &message, file_to_recv);
                break;
            case data_end:
                printf("\n\n----------------------\nFinished downloading data!\n----------------------\n\n");
                fclose(file_to_recv);    
                break;
            case measurement_control_request:
                if (receive_measurement_control_request(socket_fd, &received))
                    send_measurement_report(socket_fd, &message, &cells);
                break;
        }
    
        update_battery(socket_fd, &message, &battery);
        set_current_signal_event(&cells);
        printf("\nCurrent event: %d\n", (int)cells.current_event+1);
        printf("Battery power: %i\n", battery.power_percentage);

        sleep(1);
    }
    // Join thread
    if(pthread_join(ping_thread_id, NULL) != 0) {
        add_logf(client_log_filename, LOG_ERROR, "Failed to join a thread: ping_thread_id");
        exit(1);
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