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
