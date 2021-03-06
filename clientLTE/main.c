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
s_message received;
ue_battery battery;
pthread_mutex_t lock[2];

void signal_handler(int signum) {
    running = false;
}

void* battery_thread() {
    while(running && !check_for_shutdown(socket_fd, &received)) {
        pthread_mutex_lock(&lock[0]);
        update_battery(socket_fd, &message, &battery);
        pthread_mutex_unlock(&lock[0]);
    }

    return NULL;
}
double what_time_is_it()
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec + now.tv_nsec*1e-9;
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        add_logf(client_log_filename, LOG_ERROR, "You need to pass port number as an argument!");
        return 0;
    }
    int port_number = atoi(argv[1]);
    struct sockaddr_in server;
    //s_message message;
    s_message* message_pointer = &message;
    //ue_battery battery;
    s_cells cells;
    FILE* file_to_recv;
    pthread_t thread_id[2];
    int packets_received = 0;
    double diff_time;

    srand(time(NULL)); 

    struct sigaction s_signal;
    s_signal.sa_handler = signal_handler;
    sigemptyset(&s_signal.sa_mask);
    s_signal.sa_flags = 0;


    // INITIALIZATION BLOCK /////////////////////////////////////

    initialize_battery_life(&battery);
    initialize_cells(&cells);

    if(pthread_mutex_init(&lock[0], NULL) != 0) {
        add_logf(client_log_filename, LOG_ERROR, "Mutex initialization failed!");
        exit(1);
    }

    if(pthread_mutex_init(&lock[1], NULL) != 0) {
        add_logf(client_log_filename, LOG_ERROR, "Mutex initialization failed!");
        exit(1);
    }

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

    // Create another thread for battery
    if(pthread_create(&thread_id[0], NULL, battery_thread, NULL) != 0) {
        add_logf(client_log_filename, LOG_ERROR, "Failed to create a thread!");
        exit(1);
    }

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
                file_to_recv = fopen(message.message_value.buffer,"ab+");
                add_logf(client_log_filename, LOG_INFO, "\n\n----------------------\nStarted downloading data!\n----------------------\n\n");    
                diff_time = what_time_is_it();
                break;
            case data:
                download_data(socket_fd, &message, file_to_recv);
                packets_received++;
                break;
            case data_end:
                add_logf(client_log_filename, LOG_INFO, "\n\n----------------------\nFinished downloading data!\n----------------------\n\n");
                printf("\n-------------packets received: %d ----------------\n",packets_received);
                packets_received = 0;
                fclose(file_to_recv);
                add_logf(client_log_filename, LOG_INFO, "Downloaded in %.3lf seconds\n", what_time_is_it() - diff_time);
                break;
            case measurement_control_request:
                if (receive_measurement_control_request(socket_fd, &received))
                    send_measurement_report(socket_fd, &message, &cells);
                break;
        }
    
        set_current_signal_event(&cells);

        //printf("\nCurrent event: %d\n", (int)cells.current_event+1);
        //printf("Battery power: %i\n", battery.power_percentage);
        message.message_type = -1;
    }
    // Join thread
    if(pthread_join(thread_id[0], NULL) != 0) {
        add_logf(client_log_filename, LOG_ERROR, "Failed to join a thread!");
        exit(1);
    }
    pthread_mutex_destroy(&lock[0]);
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