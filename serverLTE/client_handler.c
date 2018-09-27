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
  json_t *shutdown_notification_json;
  char *json_str_outgoing;
  size_t json_str_len;

  shutdown_notification_json = json_object();
  json_object_set(shutdown_notification_json, "message_type", json_integer(enb_off));
  json_str_outgoing = json_dumps(shutdown_notification_json,0);
  json_str_len = strlen(json_str_outgoing);
  
  client_t* client_notified = (client_t*) value;
  write(client_notified->socket, &json_str_len, json_str_len);
  size_t written = write(client_notified->socket, json_str_outgoing, json_str_len);
  
  assert(json_str_len == written);
  free(json_str_outgoing);

  /* s_message shutdown_notification;
  memset(&shutdown_notification, 0, sizeof(shutdown_notification));
  shutdown_notification.message_type = enb_off;

  send(client_notified->socket, &shutdown_notification, sizeof(shutdown_notification), 0);
  return 0; */
}

int handle_client_inactivity(void *data, const char *key, void *value) {
  time_t current_time = time(NULL);
  client_t* current_client = (client_t*) value;
  time_t time_since_last_activity = current_time - current_client->last_activity;
  bool should_kick = (time_since_last_activity > PING_TIMEOUT);

  if (should_kick) {
    add_logf(server_log_filename, LOG_INFO, "Timeout - kicking client on socket %d", current_client->socket);
    close(current_client->socket);
    delete_client_from_hashmap(server.clients, current_client->socket);
  }
  return 0;
}