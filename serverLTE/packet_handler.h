#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#define PING_TIME_NORMAL 1
#define PING_TIME_LOW_BATTERY 2

#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

#ifndef SERVER_INIT_H
#include "server_init.h"
#endif

#ifndef HASHMAP_H
#include "../lib/hashmap.h"
#endif

#ifndef MESSAGE_H
#include "../message.h"
#endif

#ifndef CLIENT_H
#include "client.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <stdbool.h>


void handle_random_access_request(int client_socket, s_message message);
void parse_packet(int number_of_event);
rrc_config generate_rrc_config(int16_t rnti);
void send_rrc_setup(int socket);
int8_t extractPreambleIndex(int16_t ra_rnti);
void send_random_access_response(int socket, int8_t preamble_index, time_t timestamp);
void handle_low_battery_request(int client_socket);
void* pinging_in_thread(void* arg);
void send_pings();
int ping_client(void *data, const char *key, void *value);
#endif
