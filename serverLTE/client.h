#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <time.h>

#ifndef HASHMAP_H
#include "../lib/hashmap.h"
#endif

typedef struct client {
    int16_t rnti;
    int8_t preamble_index; // RAPID
    time_t first_connection_timestamp; // for C-RNTI
    time_t last_activity;
    int16_t mcc;
    int16_t mnc;
    // is stored as a key of map-element, but might be useful here
    int socket; 
} client;

hashmap* clients;

#endif