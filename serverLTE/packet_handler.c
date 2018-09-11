#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

void handle_random_access_request(int client_socket, s_message message){
  int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;
  uint8_t preamble_index = extractPreambleIndex(received_ra_rnti);
  time_t current_timestamp = time(NULL);

  send_random_access_response(client_socket, preamble_index, current_timestamp);
  save_client(client_socket, preamble_index, current_timestamp, received_ra_rnti);
  printf("Random Access response sent\n");
}

void handle_pong(int client_socket) {
  //update client's lst activity timestamp
  client_t* to_be_updated = get_client_by_socket(server.clients, client_socket);
  to_be_updated->last_activity = time(NULL);
  printf("\nGOT PONG\n");
}

void parse_packet(int number_of_event) {
  printf ("PARSE PACKET!\n");
  s_message message;
  if(read(server.events[number_of_event].data.fd, &message, sizeof(message)) == -1) {
    error("read in parse_packet");
  }

  int client_socket = server.events[number_of_event].data.fd;

  switch(message.message_type) {
    case random_access_request:
      handle_random_access_request(client_socket, message);
      break;
    case rrc_request:
      send_rrc_setup(client_socket);
      break;
    case ue_battery_low:
      handle_low_battery_request(client_socket);
      break;
    case ue_battery_high:
      handle_high_battery_request(client_socket);
    case pong:
      handle_pong(client_socket);
      break;
    default:
      break;
  }
}

rrc_config generate_rrc_config(int16_t rnti) {
  rrc_config setup;
  setup.NULRB = 9;
  setup.NSubframe = 0;
  setup.NCellID = 10;
  setup.RNTI = rnti;
  setup.cyclic_prefix = Normal;
  setup.hopping = Off;
  setup.SegGroup = 0;
  setup.CyclicShift = 0;
  setup.ShortEnd = 0;
  return setup;
}

void send_rrc_setup(int socket) {
  int16_t client_rnti = get_client_rnti(socket);
  s_message response;
  response.message_type = rrc_setup;
  response.message_value.rrc_response = generate_rrc_config(client_rnti);
  send(socket, &response, sizeof(response), 0);
  printf("sent RRC setup\n");
}

int8_t extractPreambleIndex(int16_t ra_rnti) {
  return ((ra_rnti & 0b1100000000000000) >> 8);
}

void send_random_access_response(int socket, int8_t preamble_index, time_t timestamp) {
    s_message response;
    memset(&response, 0, sizeof(response));
    response.message_type = random_access_response;
    response.message_value.message_response.rapid = preamble_index;
    response.message_value.message_response.unix_epoch_timestamp = timestamp;
    printf("sent value: %d\n", response.message_value.message_response.rapid);
    send(socket, &response, sizeof(response), 0);
}

void handle_low_battery_request(int client_socket) {
  printf("battery LOW on client: %d \n", client_socket);
  client_t* client_with_low_battery = get_client_by_socket(server.clients, client_socket);
  client_with_low_battery->battery_state = LOW;
}

void handle_high_battery_request(int client_socket) {
  printf("battery HIGH on client: %d \n", client_socket);
  client_t* client_with_high_battery = get_client_by_socket(server.clients, client_socket);
  client_with_high_battery->battery_state = OK;
}

void* pinging_in_thread(void* arg){
  send_pings();
  return NULL;
}

void send_pings() {
  bool done = false;
  hashmap_callback ping_each_client = ping_client;
  while (!done) {
    sleep(1);
    hashmap_iter(server.clients, ping_each_client, NULL);
  }
}

int ping_client(void *data, const char *key, void *value) {
  time_t current_time = time(NULL);
  int client_socket = atoi(key);
  client_t* current_client = (client_t*) value;
  time_t time_since_last_activity = current_time - current_client->last_activity;
  bool should_ping = (current_client->battery_state == OK && (time_since_last_activity > PING_TIME_NORMAL))
  || (current_client->battery_state == LOW && (time_since_last_activity > PING_TIME_LOW_BATTERY));

  if (should_ping) {
    s_message ping_message;
    ping_message.message_type = ping;
    send(client_socket, &ping_message, sizeof(ping_message), 0);
    printf("sent ping to client on socket: %d\n", client_socket);
  }
  return 0;

}
