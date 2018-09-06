#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif


void save_client(int socket, int8_t preamble_index, time_t current_timestamp) {
  client new_client;
  new_client.preamble_index = preamble_index;
  new_client.last_activity = current_timestamp;
  new_client.first_connection_timestamp = current_timestamp;
  new_client.socket = socket;
  // TODO remove conversion after key in hashmap is changed to int
  char key[8];
  sprintf(key, "%d", socket);
  hashmap_put(clients, key, &new_client);
}

void handle_random_access_request(int client_socket, s_message message){
  int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;
  uint8_t preamble_index = extractPreambleIndex(received_ra_rnti);
  time_t current_timestamp = time(NULL);

  send_random_access_response(client_socket, preamble_index, current_timestamp);
  save_client(client_socket, preamble_index, current_timestamp);
  printf("Random Access response sent\n");
}

void parse_packet(int number_of_event) {
  printf ("PARSE PACKET!\n");
  // TODO
  s_message message;
  if(read(server.events[number_of_event].data.fd, &message, sizeof(message)) == -1) {
    perror("read in parse_packet");
    exit(EXIT_FAILURE);
  }

  int client_socket = server.events[number_of_event].data.fd;
  switch(message.message_type) {
    case random_access_request:
      handle_random_access_request(client_socket, message);
      break;
    case rrc_request:
      send_rrc_setup(client_socket);
      break;
    default:
      break;
  }
}

void send_rrc_setup(int socket) {
  //TODO
}

int8_t extractPreambleIndex(int16_t ra_rnti) {
  return ((ra_rnti & 0b1100000000000000) >> 8);
}

void send_random_access_response(int socket, int8_t preamble_index, time_t timestamp) {
    s_message response;
    response.message_type = random_access_response;
    response.message_value.message_response.rapid = preamble_index;
    response.message_value.message_response.unix_epoch_timestamp = timestamp;
    printf("sent value: %d\n", response.message_value.message_response.rapid);
    send(socket, &response, sizeof(response), 0);
}