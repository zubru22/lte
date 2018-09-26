/**
@file packet_handler.h
*/
#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#define PING_TIME_NORMAL 5
#define PING_TIME_LOW_BATTERY 10

//should be bigger than PING_TIME_NORMAL and PING_TIME_LOW_BATTERY
#define PING_TIMEOUT 4000

#ifndef LOGS_H
#include "../logs/logs.h"
#endif

#ifndef SERVER_H
#include "server.h"
#endif

#ifndef HASHMAP_H
#include "../lib/hashmap.h"
#endif

#ifndef MESSAGE_H
#include "../message.h"
#endif

#ifndef CLIENT_OBJECT_H
#include "client_object.h"
#endif

#ifndef CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/stat.h>

#include <fcntl.h>

extern const int SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD;

void handle_random_access_request(int client_socket, s_message message);
/**
    @brief Called when server receives message of type pong, updates UE's last activity timestamp.
    @param[in] client_socket represents socket (file descriptor) of UE that sent pong (i.e. ping request).
*/
void handle_pong(int client_socket);
/**
    @brief Handles incoming packet and calls appropriate function according to message type.
    @param[in] client_socket Index of currently handled event, which are returned by epoll_wait
*/
void parse_packet(int number_of_event);
/**
    @brief Generates RRC configuration needed for RRC response.
    @param[in] rnti RNTI code of client
    @return Returns ready rrc_config struct to be sent in rrc_response message.
*/
rrc_config generate_rrc_config(int16_t rnti);
/**
    @brief Sends RRC setup as a response to RRC request.
    @param[in] socket Socket of UE that sent RRC request
*/
void send_rrc_setup(int socket);
/**
    @brief Extracts preamble index, which is 2 first bits of ra_rnti.
    @param[in] ra_rnti Number representing RA RNTI code of client
    @return Returns preamble index, also called RAPID.
*/
int8_t extractPreambleIndex(int16_t ra_rnti);
/**
    @brief Sends Random Access response, after UE's demand.
    @param[in] socket Socket of UE that send Random Access Request
    @param[in] preamble_index Actually 2 bit number used for establishing connection
    @param[in] timestamp To distinguish between clients with the same preamble_index
*/
void send_random_access_response(int socket, int8_t preamblel_index, time_t timestamp);
/**
    @brief Sets UE's battery state to LOW. Called as response to UE's low battery notification.
    @param[in] client_socket Socket of client who sent low battery notification
*/
void handle_low_battery_request(int client_socket);
/**
    @brief Sets UE's battery state to OK. Called as response to UE's high battery notification.
    @param[in] client_socket Socket of client who sent low battery notification
*/
void handle_high_battery_request(int client_socket);
/**
    @brief Function invoked to be run to ping and kick clients (due to inactivity) in separate thread.
    @param[in] arg Parameter of type void* required by pthread, not used
*/
void* ping_and_timeout_in_thread(void* arg);
/**
    @brief Invoked by hashmap_iter on every map element. Checks if client should be pinged and pings if necessary.
    @param[in] *data Passed to function by hashmap iter if any data needs to be used inside, unused here.
    @param[in] *key Key of currently iterated client.
    @param[in] *key Value of currently iterated client.
    @return 0 required by hashmap library (1 would stop iterating).
*/
int ping_client(void *data, const char *key, void *value);
/**
    @brief This function triggers when user sends "shutdown message". It closes client's socket and removes him from hashmap.
    @param[in] client_socket Socket of client that send shutdown notification.
*/
void handle_client_power_off(int client_socket);
/**
  @fn void* send_measurement_control_requests(void* arg)
  @brief starts while loop, which periodically iterates through clients hashmap to send measurement_control_request
  @param arg - unused param, written only for compability with pthread_create
  @return nothing is returned
*/
void* send_measurement_control_requests(void* arg);
/**
  @brief called on each client by send_measurement_control_requests, send measurement control request to chosen client
  @param data - data passed to function by hashmap iter, used only for compatibility purpose with hashmap_callback
  @param key - key of the client hashmap (socket of client)
  @param value - client_t structure stored in hashmap
  @return nothing is returned
*/
int send_measurement_control_request(void *data, const char *key, void *value);

void handle_measurement_report(int client_socket, s_message measurement_report_message);

void handle_a3_event(int client_socket);

void handle_x2ap_resource_status_request(int client_socket);

void handle_x2ap_resource_status_response(int client_socket);

void handle_x2ap_handover_request(int client_socket);

void handle_x2ap_handover_request_acknowledge(int client_socket);

void handle_handover();

int handle_resource_request(int client_socket, s_message resource_request);

void interruptible_sleep(int seconds);

#endif
