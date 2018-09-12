#ifndef SERVER_H
#include "server.h"
#endif

bool threads_done = false;
pthread_t thread_id;

void server_t__init(server_t* self, int socket, struct sockaddr_in server_address, struct epoll_event event, int epoll_file_descriptor) {
  self->socket = socket;
  self->server_address = server_address;
  self->event = event;
  self->epoll_file_descriptor = epoll_file_descriptor;
  hashmap_init(0, &self->clients);
}

int servet_t__socket(server_t* self) {
  return self->socket;
}

void server_t__destroy(server_t* self) {
  close_clients_sockets();
  hashmap_destroy(self->clients);
  close(self->socket);
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

  pthread_create(&thread_id, NULL, pinging_in_thread, NULL);
}

void receive_packets() {
  int number_of_file_descriptors_ready;
  while(1) {
      if((number_of_file_descriptors_ready = epoll_wait(server.epoll_file_descriptor, server.events, MAX_EVENTS, -1)) == -1) {
        error("epoll_wait in init_server");
      }
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
  client_t* client = (client_t*)malloc(sizeof(client_t)); // TODO
  struct sockaddr_in client_address;
  socklen_t client_length;
  client_length = sizeof(client_address);
  client->socket = accept(
                         server.socket,
                         (struct sockaddr *) &client_address,
                         &client_length);
  if (client->socket == -1) {
    error("accept in accept_client");
  }
  add_logf(server_log_filename, LOG_INFO, "ACCEPTED SOCK: %d", client->socket);
  server.event.events = EPOLLIN;
  server.event.data.fd = client->socket;
  if (epoll_ctl(
                server.epoll_file_descriptor,
                EPOLL_CTL_ADD,
                client->socket,
                &server.event
               ) == -1) {
      error("epoll_ctl in accept_client");
  }
  put_client_in_hashmap(server.clients, client->socket, client);
}

void remind_about_port() {
  add_logf(server_log_filename, LOG_ERROR, "Run program: ./server PORT_NAME");
  exit(EXIT_FAILURE);
}

void clean() {
    add_logf(server_log_filename, LOG_INFO, "CLEAN");
    threads_done = true;
    pthread_join(thread_id, NULL);
    server_t__destroy(&server);
}

void error(const char* error_message) {
  add_logf(server_log_filename, LOG_ERROR, error_message);
  if (errno != EINTR) {
    clean();
  }
  exit(EXIT_FAILURE);
}
