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

void put_client_in_hashmap(hashmap* map_of_clients, int socket, client* client_inserted) {
    char key[8];
    sprintf(key, "%d", socket);
    hashmap_put(clients, key, client_inserted);
}

void close_clients_sockets() {
    //hashmap_callback close_socket_of_each_client = close_client_socket;
    while(hashmap_iter(server.clients, close_client_socket, NULL) != 1);
}

void close_client_socket(void *data, const char *key, void *value) {
    int client_socket = atoi(key);
    close(client_socket);
}
