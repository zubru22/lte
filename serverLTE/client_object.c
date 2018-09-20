#ifndef CLIENT_OBJECT_H
#include "client_object.h"
#endif
#ifndef SERVER_H
#include "server.h"
#endif
#include <stdlib.h>

client_t* get_client_by_socket(hashmap* map_of_clients, int socket) {
    char key[8];
    sprintf(key, "%d", socket);
    void* searched_client;
    if (hashmap_get(server.clients, key, &searched_client) == -1) {
        add_logf(server_log_filename, LOG_ERROR, "error retrieving data from clients hashmap - no client %s found", key);
        return NULL;
    }

    return ((client_t*) searched_client);
}

void put_client_in_hashmap(hashmap* map_of_clients, int socket, client_t* client_inserted) {
    char key[8];
    sprintf(key, "%d", socket);
    hashmap_put(server.clients, key, client_inserted);
}

void close_clients_sockets() {
    hashmap_iter(server.clients, (hashmap_callback)close_client_socket, NULL);
}

void close_client_socket(void *data, const char *key, void *value) {
    int client_socket = atoi(key);
    close(client_socket);
}

void delete_client_from_hashmap(hashmap* map_of_clients, int socket) {
    char key[8];
    sprintf(key, "%d", socket);
    hashmap_delete(server.clients, key);
}
