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

void receive_packets() {
  int number_of_file_descriptors_ready;
  while(1) {
      if((number_of_file_descriptors_ready = epoll_wait(server->epoll_file_descriptor, server->events, MAX_EVENTS, -1)) == -1) {
        error("epoll_wait in init_server");
      }
      handle_connection(number_of_file_descriptors_ready);
  }
}

void handle_connection(int number_of_file_descriptors_ready) {
  int n;
  for (n = 0; n < number_of_file_descriptors_ready; ++n) {
      if (server->events[n].data.fd == server->socket) {
        accept_client(n);
      } else {
        char buf[128];
        memset(buf, 0, 128);
        read (server->events[n].data.fd, buf, 128);
        if (strcmp(buf, "") == 0 && errno != EAGAIN) {
          for (int it = 0; it < CLIENTS_LIMIT; it++) {
            if (newsockfd[it] == events[n].data.fd) {
              newsockfd[it] = 0;
              close (events[n].data.fd);
              break;
            }
          }
        } else {
          printf ("buf = %s\n", buf);
        }
      }
  }
}

void accept_client(int event_number) {
  int it;
  for (it = 0; it < MAX_CLIENTS; it++) {
    if (server->clients[it] == NULL) {
      server->clients[it] = (client_t*)malloc(sizeof(client_t));
      server->clients[it]->socket = accept(server->socket,
                         (struct sockaddr *) &server->clients[it]->client_address, &server->clients[it]->client_length);
      if (server->clients[it]->socket == -1) {
          perror("accept");
          exit(EXIT_FAILURE);
      }
      server->clients[it]->event.events = EPOLLIN;
      server->clients[it]->event.data.fd = server->clients[it]->socket;
      if (epoll_ctl(server->epoll_file_descriptor, EPOLL_CTL_ADD, server->clients[it]->socket,
                  &server->event) == -1) {
          perror("epoll_ctl in accept_client");
          exit(EXIT_FAILURE);
      }
      break;
    }
  }
  if (it == MAX_CLIENTS) {
    // cancel client connection
  }
}
