#ifndef CLIENT_HANDLER_H
#include "client_handler.h"
#endif

#ifndef SERVER_H
#include "server.h"
#endif

void save_client(int socket, int8_t preamble_index, time_t current_timestamp, int16_t received_ra_rnti) {
  client* new_client = (client*) malloc(sizeof(client));
  new_client->preamble_index = preamble_index;
  new_client->last_activity = current_timestamp;
  new_client->first_connection_timestamp = current_timestamp;
  new_client->socket = socket;
  new_client->rnti = received_ra_rnti;
  new_client->battery_state = OK;

  put_client_in_hashmap(clients, socket, new_client);
}

int16_t get_client_rnti(int socket) {
  return get_client_by_socket(clients, socket)->rnti;
}
