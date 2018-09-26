#ifndef CLIENT_HANDLER_H
#include "client_handler.h"
#endif

#ifndef SERVER_H
#include "server.h"
#endif

void update_client_by_ra_rnti_data(int socket, int8_t preamble_index, time_t current_timestamp, int16_t received_ra_rnti) { // TODO
  client_t* client_to_update = get_client_by_socket(server.clients, socket);
  if (client_to_update) {
    client_to_update->preamble_index = preamble_index;
    client_to_update->last_activity = current_timestamp;
    client_to_update->first_connection_timestamp = current_timestamp;
    client_to_update->rnti = received_ra_rnti;
    client_to_update->battery_state = OK;
  }
}

int16_t get_client_rnti(int socket) {
  return get_client_by_socket(server.clients, socket)->rnti;
}

int notify_client_of_shutdown(void *data, const char *key, void *value) {
  client_t* client_notified = (client_t*) value;

  s_message shutdown_notification;
  memset(&shutdown_notification, 0, sizeof(shutdown_notification));
  shutdown_notification.message_type = enb_off;

  if (send(client_notified->socket, &shutdown_notification, sizeof(shutdown_notification), 0) == -1) {
    error ("send in notify_client_of_shutdown");
  } else {
    add_logf(server_log_file, LOG_INFO, "Sent shutdown notification on socket: %d", client_notified->socket);
  }
  return 0;
}

int handle_client_inactivity(void *data, const char *key, void *value) {
  time_t current_time = time(NULL);
  client_t* current_client = (client_t*) value;
  time_t time_since_last_activity = current_time - current_client->last_activity;
  bool should_kick = (time_since_last_activity > PING_TIMEOUT);

  if (should_kick && current_client->is_server == false) {
    add_logf(server_log_file, LOG_INFO, "Timeout - kicking client on socket %d", current_client->socket);
    close(current_client->socket);
    delete_client_from_hashmap(server.clients, current_client->socket);
  }
  return 0;
}

ssize_t send_thread_safe(int client_socket, const void *buf, size_t size_of_buffer, int flags) {
  client_t* current_client = get_client_by_socket(server.clients, client_socket);
  pthread_mutex_lock(&current_client->socket_lock);
  int bytes_sent = send(client_socket, buf, size_of_buffer, flags);
  pthread_mutex_unlock(&current_client->socket_lock);
  return bytes_sent;
}
