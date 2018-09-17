#ifndef SERVER_H
#define SERVER_H

#ifndef HASHMAP_H
#include "../lib/hashmap.h"
#endif

#ifndef CLIENT_OBJECT_H
#include "client_object.h"
#endif

#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

#ifndef MESSAGE_H
#include "../message.h"
#endif

#ifndef CLIENT_OBJECT_HANDLER_H
#include "client_handler.h"
#endif

#ifndef LOGS_H
#include "../logs/logs.h"
#endif

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_LISTEN_QUERIED_CONNECTIONS 128 // value suggested as "safe" for most of the systems
#define MAX_EVENTS 100

typedef struct {
    int socket;
    int target_socket;
    struct sockaddr_in server_address;
    int epoll_file_descriptor;
    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];
    hashmap* clients;
} server_t;

server_t server;
extern bool threads_done;

void server_t__init(server_t* self, int socket, int target_socket, struct sockaddr_in server_address, struct epoll_event event, int epoll_file_descriptor);
int server_t__socket(server_t* self);
void server_t__destroy(server_t* self);
void init_server_address(struct sockaddr_in* server_address, int port);
void init_server(int port);
void receive_packets();
void handle_connection(int number_of_file_descriptors_ready);
void accept_client();
void remind_about_port();
void expand_clients();
void clean();
void error(const char* error_message);
void broadcast_shutdown_notification();

#endif
