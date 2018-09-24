/**
@file packet_handler.h
*/
#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#define PING_TIME_NORMAL 2
#define PING_TIME_LOW_BATTERY 5

//should be bigger than PING_TIME_NORMAL and PING_TIME_LOW_BATTERY
#define PING_TIMEOUT 10

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
#include <errno.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <stdbool.h>
#include <jansson.h>

extern const int SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD;

void handle_random_access_request(int client_socket, json_t *json);
void handle_pong(int client_socket);
void parse_packet(int number_of_event);
rrc_config generate_rrc_config(int16_t rnti);
void send_rrc_setup(int socket);
int8_t extractPreambleIndex(int16_t ra_rnti);
void send_random_access_response(int socket, int8_t preamble_index, time_t timestamp);
void handle_low_battery_request(int client_socket);
void handle_high_battery_request(int client_socket);
void* pinging_in_thread(void* arg);
void send_pings_handle_timeout();
int ping_client(void *data, const char *key, void *value);
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
  @param data - hashmap of clients, unused in function, used for compatibility purpose with hashmap_callback
  @param key - key to the hashmap (socket of client)
  @param value - client_t structure stored in hashmap
  @return nothing is returned
*/
int send_measurement_control_request(void *data, const char *key, void *value);
#endif
