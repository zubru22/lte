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
#ifndef DISPLAY_H
#include "display.h"
#endif
#ifndef STDIO_H
#include <stdio.h>
#endif
#ifndef STDLIB_H
#include <stdlib.h>
#endif
#ifndef CTYPE_H
#include <ctype.h>
#endif
#include <math.h>

static char client_log_filename[] = "../logs/client.log";
static char sent_messages[] = "sent_messages.txt";

volatile bool refresh = true;
volatile bool running = true;
int socket_fd;
s_message message;
s_message received;
ue_battery battery;
pthread_mutex_t lock[2];
const char* localhost = "127.0.0.1";
FILE* log_file;
char message_buff[100] = "";
bool isMessage = false;

bool downloading = false;

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

void* keyboard_thread() {
    while(running && !check_for_shutdown(socket_fd, &received)) {
            pthread_mutex_lock(&lock[1]);

            fgets(message_buff, sizeof(message_buff), stdin);
            if(isdigit((unsigned char)message_buff[0]) && isdigit((unsigned char)message_buff[8]))
            {    
                for (int i = 1; i <= 7; i++) {
                    if(isdigit((unsigned char)message_buff[i]))
                        isMessage = true;
                    else {
                        isMessage = false;
                        break;
                    }
                }
            }
            if(strcmp(message_buff,"d\n") == 0 && false == downloading) {
                downloading = true;

                if(!send_resource_request(socket_fd, &message)) {

                }
            }
            else if (isMessage) {
                if(send_SMS(socket_fd, &message, message_buff) == 0)
                    add_logf(log_file, LOG_INFO, "Message sent!");
                else
                    add_logf(log_file, LOG_INFO, "Couldn't send message!");
                
                FILE* sent_file;

                if((sent_file = fopen(sent_messages, "a")) == NULL)
                    printf("Failed to open sent messages file!\n");

                fprintf(sent_file, "Phone number: ");

                for(int i = 0; i < 8; i++)
                    fprintf(sent_file, "%c", message_buff[i]);

                fprintf(sent_file, "\n");

                for(int i = 9; i < 100; i++)
                    fprintf(sent_file, "%c", message_buff[i]);

                fprintf(sent_file, "\n");

                fclose(sent_file);
            }
            else if(strcmp(message_buff, "1\n") == 0) {
                printf("\e[1;1H\e[2J");
                menu_options = DISPLAY_LOGS;
                display_logs(log_file);
                printf("\nPress 5 - main menu\n");
            }
            else if(strcmp(message_buff, "5\n") == 0) {
                menu_options = DISPLAY_MENU;
            }

            pthread_mutex_unlock(&lock[1]);
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
    menu_options = DISPLAY_MENU;
    // Clear log file from previous session
    fclose(fopen(client_log_filename, "w"));
    // Open the file for appending
    if((log_file = fopen(client_log_filename, "a")) == NULL)
        puts("Couldn't open log file!\n");

    if(argc < 2){
        add_logf(log_file, LOG_ERROR, "You need to pass port number as an argument!");
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
    should_print_to_console = false;

    srand(time(NULL));

    struct sigaction s_signal;
    s_signal.sa_handler = signal_handler;
    sigemptyset(&s_signal.sa_mask);
    s_signal.sa_flags = 0;

    // INITIALIZATION BLOCK /////////////////////////////////////
    initialize_battery_life(&battery);
    initialize_cells(&cells);

    if(pthread_mutex_init(&lock[0], NULL) != 0) {
        add_logf(log_file, LOG_ERROR, "Mutex initialization failed!");
        exit(1);
    }

    if(pthread_mutex_init(&lock[1], NULL) != 0) {
        add_logf(log_file, LOG_ERROR, "Mutex initialization failed!");
        exit(1);
    }

    //init_connection returns 0 on error, else function returns 1
    if (init_connection(&socket_fd, &server, port_number, localhost)) {
        add_logf(log_file, LOG_SUCCESS, "Connected!");
    }
    else {
        add_logf(log_file, LOG_ERROR, "Failed to connect!");
        return 0;
    }

    //returns -1 on error, else 0
    if (send_prach_preamble(socket_fd, &message, generate_ra_rnti) == -1) {
        add_logf(log_file, LOG_ERROR, "Failed to send preamble!");
        return 0;
    }
    else {
        add_logf(log_file, LOG_SUCCESS, "Message sent!");

        if (message.message_type == random_access_request) {
            add_logf(log_file, LOG_INFO, "Message type: RA_RNTI.");
        }
        else {
            add_logf(log_file, LOG_WARNING, "Message type: NOT RA_RNTI.");
        }
        add_logf(log_file, LOG_INFO, "RA_RNTI VALUE: %d", message.message_value.message_preamble.ra_rnti);
    }

    int prach_response_func_status = receive_prach_response(socket_fd, &received, &message);

    if(-1 == prach_response_func_status) {
        add_logf(log_file, LOG_ERROR, "Error on recv()!");
        return 0;
    }
    else if(1 == prach_response_func_status) {
        add_logf(log_file, LOG_ERROR, "Response not OK!");
        return 0;
    }
    else {
        add_logf(log_file, LOG_SUCCESS, "Response type OK.");
        add_logf(log_file, LOG_SUCCESS, "RACH Success!");
    }

    if(send_rrc_connection_request(socket_fd, &message, generate_ue_identity) == -1) {
        add_logf(log_file, LOG_ERROR, "Failed to send RRC connection request!");
        return 0;
    }
    else {
        add_logf(log_file, LOG_SUCCESS, "Successfully sent RRC connection request.");
    }

    receive_rrc_setup(socket_fd, &received, &message);
    sigaction(SIGINT, &s_signal, NULL);

    // Create another thread for battery
    if(pthread_create(&thread_id[0], NULL, battery_thread, NULL) != 0) {
        add_logf(log_file, LOG_ERROR, "Failed to create a thread!");
        exit(1);
    }

    // Create another thread for keyboard
    if(pthread_create(&thread_id[1], NULL, keyboard_thread, NULL) != 0) {
        add_logf(log_file, LOG_ERROR, "Failed to create a thread!");
        exit(1);
    }

    double time_start = what_time_is_it();
    double time_end;
    // While running and have not received eNodeB shutdown message
    while (running && !check_for_shutdown(socket_fd, &received)) {
        recv(socket_fd, (s_message*)message_pointer, sizeof(message), MSG_DONTWAIT);
        switch (message.message_type) {
            case ping:
                if (receive_ping(socket_fd, &message) == 0) {
                    if (send_pong(socket_fd, &message) == -1)
                            add_logf(log_file, LOG_ERROR, "Failed to response to server ping!");
                        else {
                            decrease_after_ping(socket_fd, &message, &battery);
                            add_logf(log_file, LOG_SUCCESS, "Successfully handled server ping!");
                        }
                }
                break;
            case data_start:                
                file_to_recv = fopen(message.message_value.buffer,"ab+");
                add_logf(log_file, LOG_INFO, "\n\n----------------------\nStarted downloading data!\n----------------------\n\n");
                diff_time = what_time_is_it();
                break;
            case data:
                download_data(socket_fd, &message, file_to_recv);
                packets_received++;
                break;
            case data_end:
                add_logf(log_file, LOG_INFO, "\n\n----------------------\nFinished downloading data!\n----------------------\n\n");
                printf("\n-------------packets received: %d ----------------\n",packets_received);
                packets_received = 0;
                fclose(file_to_recv);
                add_logf(log_file, LOG_INFO, "Downloaded in %.3lf seconds\n", what_time_is_it() - diff_time);
                downloading = false;
                break;
            case measurement_control_request:
                send_measurement_report(socket_fd, &message, &cells);
                break;
            case resource_response:
                if (message.message_value.resource_state) {
                    add_logf(log_file, LOG_INFO, "Resource avaliable.");
                }
                else
                    add_logf(log_file, LOG_WARNING, "Resource unavailable. Download won't be conducted.");
                break;
            case rrc_connection_reconfiguration_request:
                add_logf(log_file, LOG_WARNING, "rrc_connection_reconfiguration_request SUCCESS");
                send_ue_off_signal(socket_fd, &message);
                close(socket_fd);
                if (init_connection(&socket_fd, &server, message.message_value.handover_request.port, message.message_value.handover_request.ip_address)) {
                    add_logf(log_file, LOG_SUCCESS, "Connected!");
                }
                else {
                    add_logf(log_file, LOG_ERROR, "Failed to connect!");
                    return 0;
                }
                break;
            case SMS:
                add_logf(log_file, LOG_INFO, "Received message: %s", message.message_value.text_message);
                    
                break;
            default:
                break;
        }
        set_current_signal_event(&cells);
        if(menu_options == DISPLAY_MENU && refresh) { 
            refresh = false;
            printf("\e[1;1H\e[2J");
            printf("Battery: %i%%\t\t\t\t\t\t", battery.power_percentage);
            printf("Signal power eNodeB(1): %d\n", cells.cells_signals[0].rsrp);
            printf("\t\t\t\t\t\t        Signal power eNodeB(2): %d", cells.cells_signals[1].rsrp);
            printf("\n\n\n");
            display_menu();
        }
        time_end = what_time_is_it();
        if((time_end - time_start) > 1.0) {
            refresh = true;
            time_start = time_end;
        }

        message.message_type = -1;
    }
    // Join thread
    if(pthread_join(thread_id[0], NULL) != 0) {
        add_logf(log_file, LOG_ERROR, "Failed to join a thread!");
        exit(1);
    }
    if(pthread_join(thread_id[1], NULL) != 0) {
        add_logf(log_file, LOG_ERROR, "Failed to join a thread!");
        exit(1);
    }
    for(int i = 0; i < 2; i++)
        pthread_mutex_destroy(&lock[i]);
    // Check if eNodeB is still on before trying to send UE off signal
    if (message.message_type != enb_off)
    {
        if(-1 == send_ue_off_signal(socket_fd, &message))
            add_logf(log_file, LOG_ERROR, "Client failed to send ue_off notification!");
        else
            add_logf(log_file, LOG_SUCCESS, "Client successfully disconnected from server!");
    }
    fclose(log_file);
    return 0;
}
