#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

void handle_random_access_request(int client_socket, s_message message){
  int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;
  uint8_t preamble_index = extractPreambleIndex(received_ra_rnti);
  time_t current_timestamp = time(NULL);

  send_random_access_response(client_socket, preamble_index, current_timestamp);
  save_client(client_socket, preamble_index, current_timestamp, received_ra_rnti);
  add_logf(server_log_filename, LOG_INFO, "Random Access response sent");
}

void handle_pong(int client_socket) {
  //update client's lst activity timestamp
  client* to_be_updated = get_client_by_socket(clients, client_socket);
  to_be_updated->last_activity = time(NULL);
  add_logf(server_log_filename, LOG_INFO, "Received pong");
}

void parse_packet(int number_of_event) {
  int client_socket = server.events[number_of_event].data.fd;

  add_logf(server_log_filename, LOG_INFO, "Parsing packet from socket: %d", client_socket);
  s_message message;
  int number_of_bytes_read = read(client_socket, &message, sizeof(message));
  if (number_of_bytes_read == -1) {
    error("read in parse_packet");
  } else if (number_of_bytes_read == 0) {
    add_logf(server_log_filename, LOG_INFO, "Client disconnected: %d", client_socket);
    close(client_socket);
    delete_client_from_hashmap(clients, client_socket);
  }

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
  add_logf(server_log_filename, LOG_INFO, "Sent RRC setup");
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
    add_logf(server_log_filename, LOG_INFO, "Sent value: %d", preamble_index);
    send(socket, &response, sizeof(response), 0);
}

void handle_low_battery_request(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "Battery LOW on client: %d", client_socket);
  client* client_with_low_battery = get_client_by_socket(clients, client_socket);
  client_with_low_battery->battery_state = LOW;
}

void handle_high_battery_request(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "Battery HIGH on client: %d", client_socket);
  client* client_with_high_battery = get_client_by_socket(clients, client_socket);
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
    hashmap_iter(clients, ping_each_client, NULL);
  }
}

int ping_client(void *data, const char *key, void *value) {
  time_t current_time = time(NULL);
  int client_socket = atoi(key);
  client* current_client = (client*) value;
  time_t time_since_last_activity = current_time - current_client->last_activity;
  bool should_ping = (current_client->battery_state == OK && (time_since_last_activity > PING_TIME_NORMAL))
  || (current_client->battery_state == LOW && (time_since_last_activity > PING_TIME_LOW_BATTERY)); 

  if (should_ping) {
    s_message ping_message;
    ping_message.message_type = ping;
    send(client_socket, &ping_message, sizeof(ping_message), 0);
    add_logf(server_log_filename, LOG_INFO, "Sent ping to client on socket: %d", client_socket);
  }
  return 0;

}
