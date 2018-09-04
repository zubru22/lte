#include "server_init.h"

void server_t__init(server_t* self, int socket, struct sockaddr server_address) {
  self->socket = socket;
  self->server_address = server_address;
  int i;
  for(i = 0; i < MAX_CLIENTS; i++) {
    self->clients[i] = NULL;
  }
}

int servet_t__socket(server_t* self) {
  return self->socket;
}

void init_server_address(struct sockaddr_in* server_address, int port) {
  bzero((char *) server_address, sizeof(*server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);
}

void init_server(int port) {
  int server_socket;
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    error("socket in init_server");
  }
  struct sockaddr_in server_address;
  init_server_address(&server_address, port);
  if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
    error("bind in init_server");
  }
  if (listen(server_socket, MAX_CLIENTS) == -1) {
    error ("listen in init_server");
  }
  struct epoll_event event, events[MAX_EVENTS];
  int epoll_file_descriptor;
  if ((epoll_file_descriptor = epoll_create1(0)) == -1) {
    error("epoll_create1 in init_server");
  }
  event.events = EPOLLIN;
  event.data.fd = server_socket;
  if (epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, server_socket, &event) == -1) {
    close(epoll_file_descriptor);
    error("epoll_ctl in init_server");
  }
  server_t__init(&server, server_socket, server_address);
}
