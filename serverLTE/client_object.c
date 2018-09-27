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
    pthread_mutex_lock(&server.hashmap_lock);
    if (hashmap_get(server.clients, key, &searched_client) == -1) {
        add_logf(server_log_file, LOG_ERROR, "error retrieving data from clients hashmap - no client %s found", key);
        pthread_mutex_unlock(&server.hashmap_lock);
        return NULL;
    } else {
        pthread_mutex_unlock(&server.hashmap_lock);
        return ((client_t*) searched_client);
    }
}

// used as hashmap-callback iterating function
int check_client_by_MSIN(void *data, const char *key, void *value) {
    int* searched_MSIN = (int*) data;
    client_t* current_client = (client_t*) value;
    if(current_client->is_server) {
        return 0;
    }
    printf("\nCurrently iterated MSIN: %d\n", current_client->phone_number);
    if (current_client->phone_number == *searched_MSIN) {
        data = current_client;
        return current_client->socket;
    } else {
        return 0;
    }
}

//returns 0 if not found, client's socket otherwise
int get_clients_socket_by_MSIN(hashmap* map_of_clients, int MSIN) {
    void* data = (void*) &MSIN;
    return hashmap_iter(server.clients, (hashmap_callback) check_client_by_MSIN, data);
}

void put_client_in_hashmap(hashmap* map_of_clients, int socket, client_t* client_inserted) {
    char key[8];
    sprintf(key, "%d", socket);
    pthread_mutex_lock(&server.hashmap_lock);
    hashmap_put(server.clients, key, client_inserted);
    pthread_mutex_unlock(&server.hashmap_lock);
}

void close_clients_sockets() {
    hashmap_iter(server.clients, (hashmap_callback)close_client_socket, NULL);
}

void close_client_socket(void *data, const char *key, void *value) {
    if(&((client_t*) value)->is_server) {
        return;
    }
    int client_socket = atoi(key);
    pthread_mutex_lock( &((client_t*) value)->socket_lock);
    close(client_socket);
    pthread_mutex_unlock( &((client_t*) value)->socket_lock);
}

void delete_client_from_hashmap(hashmap* map_of_clients, int socket) {
    client_t* client_being_deleted;
    char key[8];

    // blocks for readability
    pthread_mutex_lock(&server.hashmap_lock);
    {
        client_being_deleted = get_client_by_socket(server.clients, socket);

        pthread_mutex_lock(&client_being_deleted->socket_lock);
        {
            close(client_being_deleted->socket);
        }
        pthread_mutex_unlock(&client_being_deleted->socket_lock);

        pthread_mutex_destroy(&client_being_deleted->socket_lock);
        sprintf(key, "%d", socket);
        hashmap_delete(server.clients, key);
    }
    pthread_mutex_unlock(&server.hashmap_lock);
}
