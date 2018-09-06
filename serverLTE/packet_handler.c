#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif



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
      send_random_access_response(client_socket, message);
      printf("Random Access response sent\n");
      break;
    case rrc_request:
      send_rrc_setup(client_socket)
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

void send_random_access_response(int socket, s_message message)
{
    int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;

    s_message response;
    response.message_type = random_access_response;
    response.message_value.message_response.rapid = extractPreambleIndex(received_ra_rnti);
    response.message_value.message_response.unix_epoch_timestamp = time(NULL);
    printf("sent value: %d\n", response.message_value.message_response.rapid);
    send(socket, &response, sizeof(response), 0);
}