#ifndef SERVER_H
#include "server.h"
#endif

bool threads_done = false;
pthread_t pinging_in_thread_id;
pthread_t send_measurement_control_requests_id;

void server_t__init(server_t* self, int socket, int target_port, struct sockaddr_in server_address, struct epoll_event event, int epoll_file_descriptor) {
  self->socket = socket;
  self->target_socket = -2; // indicates that server did not connect to target
  self->target_port = target_port;
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
  pthread_mutex_lock(&server.hashmap_lock);
  hashmap_destroy(self->clients);
  pthread_mutex_unlock(&server.hashmap_lock);
  close(self->socket);
}

void init_server_address(struct sockaddr_in* server_address, int port) {
  bzero((char *) server_address, sizeof(*server_address));
  server_address->sin_family = AF_INET;
  server_address->sin_addr.s_addr = INADDR_ANY;
  server_address->sin_port = htons(port);
}

void init_server(int port, int target_port) {
  int server_socket;
  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    error("socket in init_server");
  }
  // reuse old socket if still exists in kernel
  // see https://stackoverflow.com/a/10651048
  int value_set = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &value_set, sizeof(int));

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
  server_t__init(&server, server_socket, target_port, server_address, event, epoll_file_descriptor);

  pthread_create(&pinging_in_thread_id, NULL, ping_and_timeout_in_thread, NULL);
  pthread_create(&send_measurement_control_requests_id, NULL, send_measurement_control_requests, NULL);

  // trying to send example file to all clients:
  //char* file_to_be_sent = "tekst.txt";
  char* file_to_be_sent = "obrazek.png";
  //char* file_to_be_sent = "piesel.jpg";
  //char* file_to_be_sent = "piksel.bmp";

  if (pthread_mutex_init(&server.hashmap_lock, NULL) != 0)
    {
        error("Hashmap mutex init failed");
    }

  server_log_file = log_init(server_log_filename, "w+");
  if(server_log_file == NULL) {
    printf("Unable to open log file, exiting\n");
    exit(EXIT_FAILURE);
  }
  set_console_logging(true);
}

void receive_packets() {
  int number_of_file_descriptors_ready;
  while(1) {
      if((number_of_file_descriptors_ready = epoll_wait(server.epoll_file_descriptor, server.events, MAX_EVENTS, -1)) == -1) {
        warning("epoll_wait in init_server");
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
  client_t* client = (client_t*)malloc(sizeof(client_t));
  if (pthread_mutex_init(&client->socket_lock, NULL) != 0) {
        error("Client mutex init failed");
    }
  struct sockaddr_in client_address;
  socklen_t client_length;
  client_length = sizeof(client_address);
  client->socket = accept(
                         server.socket,
                         (struct sockaddr *) &client_address,
                         &client_length);
  client->is_server = false;
  if (client->socket == -1) {
    error("accept in accept_client");
  }
  add_logf(server_log_file, LOG_INFO, "ACCEPTED SOCK: %d", client->socket);
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
  client->last_activity = time(NULL);
  put_client_in_hashmap(server.clients, client->socket, client);
}

void remind_about_port() {
  error ("Run program: ./server PORT TARGET_PORT");
}

void broadcast_shutdown_notification() {
  add_logf(server_log_file, LOG_INFO, "Broadcasting shutdown notification");
  sleep(1);
  hashmap_iter(server.clients, (hashmap_callback) notify_client_of_shutdown, NULL);
}

void clean() {
    // disable "ctrl+c" handling so as not to invoke "clean" multiple times
    signal(SIGINT, NULL);

    add_logf(server_log_file, LOG_INFO, "CLEAN");
    threads_done = true;
    pthread_join(pinging_in_thread_id, NULL);
    add_logf(server_log_file, LOG_INFO, "Joined pinging thread");
    pthread_join(send_measurement_control_requests_id, NULL);
    add_logf(server_log_file, LOG_INFO, "Joined measurement-handling thread");    
    broadcast_shutdown_notification();
    add_logf(server_log_file, LOG_INFO, "Finished broadcasting shutdown notification");    
    pthread_mutex_destroy(&server.hashmap_lock);
    add_logf(server_log_file, LOG_INFO, "Destroyed hashmap mutex");    
    server_t__destroy(&server);
    add_logf(server_log_file, LOG_INFO, "Destroyed server");    
    add_logf(server_log_file, LOG_INFO, "Closing logging file and exiting...");    
    fclose(server_log_file);
    exit(0);
}

void error(const char* error_message) {
  add_logf(server_log_file, LOG_ERROR, error_message);
  if (errno != EINTR) {
    clean();
  }
  exit(EXIT_FAILURE);
}

void warning(const char* warning_message) {
  add_logf(server_log_file, LOG_WARNING, warning_message);
}

void connect_to_target_server() {
  if ((server.target_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    error("socket in connect_to_target_server");
  }
  printf ("SOCKET MADE: %d\n", server.target_socket);
  struct hostent* target_host_entity;
  if ((target_host_entity = gethostbyname(HOST_NAME)) == NULL) {
    error("gethostbyname in connect_to_target_server");
  }
  struct sockaddr_in target_server_address;
  bzero((char*)&target_server_address, sizeof(target_server_address));
  target_server_address.sin_family = AF_INET;
  bcopy((char *)target_host_entity->h_addr,
       (char *)&target_server_address.sin_addr.s_addr,
       target_host_entity->h_length);
  target_server_address.sin_port = htons(server.target_port);
  if (connect(
              server.target_socket,
              (struct sockaddr *) &target_server_address,
              sizeof(target_server_address)
              ) == -1) {
      error("ERROR connecting");
  }
  printf ("CONNECTED");
}

void forward_sms_message(s_message message_to_send) {
  char temporary_phone_number[9];
  memset(temporary_phone_number, 0, 9);
  strncpy(temporary_phone_number, message_to_send.message_value.text_message, 9);
  int phone_number = atoi(temporary_phone_number);
  add_logf(server_log_file, LOG_INFO, "Receiver's phone number: %d", phone_number);
  int receivers_socket = get_clients_socket_by_MSIN(server.clients, phone_number);
  client_t* client_to_send_message = get_client_by_socket(server.clients, receivers_socket);
  add_logf(server_log_file, LOG_INFO, "Actual phone number: %d", client_to_send_message->phone_number);
  if(client_to_send_message == NULL) {
    printf(" \n CLIENT (RECEIVER) NOT FOUND \n");
  }
  if (client_to_send_message) {
    if (send(client_to_send_message->socket, &message_to_send, sizeof(message_to_send), 0) == -1) {
      error("send in forward_sms_message");
    } else { 
      add_logf(server_log_file, LOG_SUCCESS, "Successfully forwarded message");
    }
  } else {
    // logic to send number to another eNB
  }
}
