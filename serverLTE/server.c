#ifndef SERVER_H
#include "server.h"
#endif



void server_t__init(server_t* self, int socket, struct sockaddr_in server_address, struct epoll_event event, int epoll_file_descriptor) {
  self->socket = socket;
  self->server_address = server_address;
  self->number_of_clients = 0;
  self->max_number_of_clients = 2;
  int i;
  self->clients = (client_t**)malloc(sizeof(client_t*) * self->max_number_of_clients);
  for (i = 0; i < self->max_number_of_clients;   i++) {
    self->clients[i] = NULL;
  }
  self->event = event;
  self->epoll_file_descriptor = epoll_file_descriptor;
}

int servet_t__socket(server_t* self) {
  return self->socket;
}

void server_t__destroy(server_t* self) {
  int i;
  for (i = 0; i < self->max_number_of_clients; i++) {
    if (self->clients[i] == NULL) {
      continue;
    }
    close(self->clients[i]->socket);
    free(self->clients[i]);
  }
  close(self->socket);
  free(self->clients);
}

void init_server_address(struct sockaddr_in* server_address, int port) {
  bzero((char *) server_address, sizeof(*server_address));
  server_address->sin_family = AF_INET;
  server_address->sin_addr.s_addr = INADDR_ANY;
  server_address->sin_port = htons(port);
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
  if (listen(server_socket, MAX_LISTEN_QUERIED_CONNECTIONS) == -1) {
    error ("listen in init_server");
  }
  struct epoll_event event;
  int epoll_file_descriptor;
  if ((epoll_file_descriptor = epoll_create1(0)) == -1) {
    error("epoll_create1 in init_server");
  }
  memset(&event, 0, sizeof(event));
  event.events = EPOLLIN;
  event.data.fd = server_socket;
  if (epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, server_socket, &event) == -1) {
    close(epoll_file_descriptor);
    error("epoll_ctl in init_server");
  }
  server_t__init(&server, server_socket, server_address, event, epoll_file_descriptor);

  hashmap_init(0, &clients);
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, pinging_in_thread, NULL);
}

void receive_packets() {
  int number_of_file_descriptors_ready;
  while(1) {
      if((number_of_file_descriptors_ready = epoll_wait(server.epoll_file_descriptor, server.events, MAX_EVENTS, -1)) == -1) {
        error("epoll_wait in init_server");
      }
      printf("number of file descriptors ready: %d\n", number_of_file_descriptors_ready);
      handle_connection(number_of_file_descriptors_ready);
  }
}

void handle_connection(int number_of_file_descriptors_ready) {
  int n;
  for (n = 0; n < number_of_file_descriptors_ready; ++n) {
      if (server.events[n].data.fd == server.socket) {
        accept_client();
      } else {
        parse_packet(n);
      }
  }
}

void accept_client() {
  if (server.number_of_clients == server.max_number_of_clients) {
    expand_clients();
  }
  int it;
  for (it = 0; it < server.max_number_of_clients; it++) {
    if (server.clients[it] == NULL) {
      server.number_of_clients++;
      server.clients[it] = (client_t*)malloc(sizeof(client_t));
      server.clients[it]->client_length = sizeof(server.clients[it]->client_address);
      server.clients[it]->socket = accept(
                                    server.socket,
                                    (struct sockaddr *) &server.clients[it]->client_address,
                                    &server.clients[it]->client_length);
      if (server.clients[it]->socket == -1) {
          error("accept in accept_client");
      }
      add_logf(server_log_filename, LOG_SUCCESS, "ACCEPTED SOCK: %d", server.clients[it]->socket);
      server.event.events = EPOLLIN;
      server.event.data.fd = server.clients[it]->socket;
      if (epoll_ctl(server.epoll_file_descriptor, EPOLL_CTL_ADD, server.clients[it]->socket,
                  &server.event) == -1) {
          error("epoll_ctl in accept_client");
      }
      break;
    }
  }
}

void remind_about_port() {
  add_logf(server_log_filename, LOG_ERROR, "Run program: ./server PORT_NAME");
  exit(EXIT_FAILURE);
}

void expand_clients() {
  client_t *temporary_clients = (client_t*)malloc(sizeof(client_t) * server.max_number_of_clients);
  int i;
  for (i = 0; i < server.max_number_of_clients; i++) {
    temporary_clients[i] = *server.clients[i];
  }
  for (i = 0; i < server.max_number_of_clients; i++) {
    free(server.clients[i]);
  }
  free(server.clients);
  server.clients = (client_t**)malloc(sizeof(client_t*) * 2 * server.max_number_of_clients);
  for (i = 0; i < server.max_number_of_clients; i++) {
    server.clients[i] = (client_t*)malloc(sizeof(client_t));
    *server.clients[i] = temporary_clients[i];
  }
  server.max_number_of_clients *= 2;
  free(temporary_clients);
}

void clean() {
    add_logf(server_log_filename, LOG_INFO, "CLEAN");
    server_t__destroy(&server);
    hashmap_destroy(clients);
}

void error(const char* error_message) {
  add_logf(server_log_filename, LOG_ERROR, error_message);

  if (errno != EINTR) {
    clean();
  }
  exit(EXIT_FAILURE);
}
