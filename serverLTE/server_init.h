#ifndef SERVER_INIT_H
#define SERVER_INIT_H

#ifndef SERVER_INIT_H
#include "server_init.h"
#endif

#ifndef HASHMAP_H
#include "../lib/hashmap.h"
#endif

#include "../message.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVENTS 100
#define MAX_CLIENTS 10

typedef struct {
    int socket;
    struct sockaddr_in client_address;
    socklen_t client_length;
} client_t;

typedef struct {
    int socket;
    struct sockaddr_in server_address;
    int epoll_file_descriptor;
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    client_t** clients;
} server_t;

server_t server;

void server_t__init(server_t* self, int socket, struct sockaddr_in server_address, struct epoll_event event, int epoll_file_descriptor);
int servet_t__socket(server_t* self);
void init_server_address(struct sockaddr_in* server_address, int port);
void init_server(int port);
void receive_packets();
void handle_connection(int number_of_file_descriptors_ready);
void accept_client();

#endif
