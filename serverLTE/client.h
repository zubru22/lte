#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <time.h>
#include <stdio.h>

#ifndef HASHMAP_H
#include "../lib/hashmap.h"
#endif

typedef enum {
  OK,
  LOW
} battery_state_t;

typedef struct client {
    int16_t rnti;
    int8_t preamble_index; // RAPID
    time_t first_connection_timestamp; // for C-RNTI
    time_t last_activity;
    int16_t mcc;
    int16_t mnc;
    battery_state_t battery_state;
    // is stored as a key of map-element, but might be useful here
    int socket;
} client;

hashmap* clients;

client* get_client_by_socket(hashmap* map_of_clients, int socket);
void put_client_in_hashmap(hashmap* map_of_clients, int socket, client* client_inserted);
#endif