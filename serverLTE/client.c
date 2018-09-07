#ifndef CLIENT_H
#include "client.h"
#endif

client* get_client_by_socket(hashmap* map_of_clients, int socket) {
    char key[8];
    sprintf(key, "%d", socket);
    void* searched_client;

    if (hashmap_get(clients, key, &searched_client) == -1) {
        printf("error retrieving data from clients hashmap - no client %s found\n", key);
        return NULL;
    }
    return ((client*) searched_client);
}