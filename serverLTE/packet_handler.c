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
  switch(message.message_type) {
    case RA_RNTI:
      send_CRNTI(server.events[number_of_event].data.fd, message);
      printf("CA_RNTI response sent\n");
      break;
    case C_RNTI:
      //TODO
      printf ("C_RNTI\n");
      break;
    default:
      break;
  }
}

int8_t extractPreambleIndex(int16_t ra_rnti) {
  return ((ra_rnti & 0b1100000000000000) >> 8);
}

void send_CRNTI(int socket, s_message message)
{
    int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;

    s_message response;
    response.message_type = C_RNTI;
    response.message_value.message_response.rapid = extractPreambleIndex(received_ra_rnti);
    response.message_value.message_response.unix_epoch_timestamp = time(NULL);
    printf("sent value: %d\n", response.message_value.message_response.rapid);
    send(socket, &response, sizeof(response), 0);
}