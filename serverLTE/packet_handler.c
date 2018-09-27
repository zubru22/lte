#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

const int SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD = 3;

void handle_random_access_request(int client_socket, json_t *json) {
  int16_t received_ra_rnti;
  json_unpack(json_object_get(json, "ra_rnti"), "i", &received_ra_rnti);
  uint8_t preamble_index = extractPreambleIndex(received_ra_rnti);
  time_t current_timestamp = time(NULL);

  send_random_access_response(client_socket, preamble_index, current_timestamp);
  update_client_by_ra_rnti_data(client_socket, preamble_index, current_timestamp, received_ra_rnti);
  add_logf(server_log_filename, LOG_INFO, "Random Access response sent");
}

void handle_pong(int client_socket) {
  //update client's lst activity timestamp
  client_t* to_be_updated = get_client_by_socket(server.clients, client_socket);
  to_be_updated->last_activity = time(NULL);
  add_logf(server_log_filename, LOG_INFO, "Received pong");
}

void handle_client_power_off(int client_socket) {
  close(client_socket);
  delete_client_from_hashmap(server.clients, client_socket);
}


void parse_packet(int number_of_event) {
  int client_socket = server.events[number_of_event].data.fd;
  int number_of_bytes_read;
  size_t json_str_len;
  char* json_str_incoming;
  json_t *json_obj;
  int message_type_received;
  

  add_logf(server_log_filename, LOG_INFO, "Parsing packet from socket: %d", client_socket);
  // Read length of json from the socket
  number_of_bytes_read = read(client_socket, (size_t*) &json_str_len, sizeof(json_str_len));

  // Abort in case of an error or zero length
  if (number_of_bytes_read == -1) {
    error("read in parse_packet");
  } else if (number_of_bytes_read == 0) {
    add_logf(server_log_filename, LOG_INFO, "Client disconnected: %d", client_socket);
    close(client_socket);
    delete_client_from_hashmap(server.clients, client_socket);
    return;
  }

  // Allocate memory for json string
  json_str_incoming = (char*) malloc(json_str_len);

  // Read json string from socket - repeat in case it's needed
  do {
    number_of_bytes_read = read(client_socket, json_str_incoming, json_str_len);
  } while (number_of_bytes_read != json_str_len);
  
  printf("should read: %lu, read: %d\n", json_str_len, number_of_bytes_read);
  printf("%s\n", json_str_incoming);

  // Decode json string to json object
  json_obj = json_loads(json_str_incoming, 0, 0);
  // String not needed any more
  free(json_str_incoming);

  // Exctract message type value from json
  json_unpack(json_object_get(json_obj, "message_type"), "i", &message_type_received);

  switch(message_type_received) {
    case random_access_request:
      handle_random_access_request(client_socket, json_obj);
      break;
    case rrc_request:
      send_rrc_setup(client_socket);
      break;
    case ue_battery_low:
      handle_low_battery_request(client_socket);
      break;
    case ue_battery_high:
      handle_high_battery_request(client_socket);
      break;
    case pong:
      handle_pong(client_socket);
      break;
    case ue_off:
      handle_client_power_off(client_socket);
      break;
    default:
      break;
  }
}

json_t* generate_rrc_config(int16_t rnti) {
  json_t *setup;
  setup = json_object();
  json_object_set(setup, "message_type", json_integer(rrc_setup));
  json_object_set(setup, "NULRB", json_integer(9));
  json_object_set(setup, "NSubframe", json_integer(0));
  json_object_set(setup, "NCellID", json_integer(10));
  json_object_set(setup, "RNTI", json_integer(rnti));
  json_object_set(setup, "cyclic_prefix", json_integer(Normal));
  json_object_set(setup, "hopping", json_integer(Off));
  json_object_set(setup, "SegGroup", json_integer(0));
  json_object_set(setup, "Cyclic_Shift", json_integer(0));
  json_object_set(setup, "ShortEnd", json_integer(0));
  return setup;
}

void send_rrc_setup(int socket) {
  json_t *rrc_setup_json;
  size_t json_str_len;
  char *json_str_outgoing;
  int16_t client_rnti = get_client_rnti(socket);
  rrc_setup_json = generate_rrc_config(client_rnti);
  json_str_outgoing = json_dumps(rrc_setup_json,0);
  json_str_len = strlen(json_str_outgoing);
  write(socket, &json_str_len, json_str_len);
  size_t written = write(socket, json_str_outgoing, json_str_len);
  assert(json_str_len == written);
  free(json_str_outgoing);
  add_logf(server_log_filename, LOG_INFO, "Sent RRC setup");
}

int8_t extractPreambleIndex(int16_t ra_rnti) {
  return ((ra_rnti & 0b1100000000000000) >> 8);
}

void send_random_access_response(int socket, int8_t preamble_index, time_t timestamp) {
    json_t *response_json;
    char *json_str_outgoing;
    size_t json_str_len;

    response_json = json_object();
    json_object_set(response_json, "message_type", json_integer(random_access_response));
    json_object_set(response_json, "rapid", json_integer(preamble_index));
    json_object_set(response_json, "timestamp", json_integer(timestamp));
    
    json_str_outgoing = json_dumps(response_json,0);
    printf("Write: %s\n", json_str_outgoing);
    
    json_str_len = strlen(json_str_outgoing);
    write(socket, &json_str_len, json_str_len);
    //send json_struckt
    size_t written = write(socket, json_str_outgoing, json_str_len);
    
    printf("Should write: %ld, written: %ld\n", json_str_len, written);
    assert(json_str_len == written);
    
    free(json_str_outgoing);
    add_logf(server_log_filename, LOG_INFO, "Sent value: %d", preamble_index);
}

void handle_low_battery_request(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "Battery LOW on client: %d", client_socket);
  client_t* client_with_low_battery = get_client_by_socket(server.clients, client_socket);
  client_with_low_battery->battery_state = LOW;
}

void handle_high_battery_request(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "Battery HIGH on client: %d", client_socket);
  client_t* client_with_high_battery = get_client_by_socket(server.clients, client_socket);
  client_with_high_battery->battery_state = OK;
}

void* pinging_in_thread(void* arg) {
  send_pings_handle_timeout();
  return NULL;
}

void send_pings_handle_timeout() {
  while (!threads_done) {
    sleep(1);
    hashmap_iter(server.clients, (hashmap_callback) ping_client, NULL);
    hashmap_iter(server.clients, (hashmap_callback) handle_client_inactivity, NULL);
  }
}

int ping_client(void *data, const char *key, void *value) {
  time_t current_time = time(NULL);
  client_t* current_client = (client_t*) value;
  time_t time_since_last_activity = current_time - current_client->last_activity;
  bool should_ping = (current_client->battery_state == OK && (time_since_last_activity > PING_TIME_NORMAL))
  || (current_client->battery_state == LOW && (time_since_last_activity > PING_TIME_LOW_BATTERY));

  if (should_ping) {
    s_message ping_message;
    memset(&ping_message, 0, sizeof(ping_message));
    ping_message.message_type = ping;
    send(current_client->socket, &ping_message, sizeof(ping_message), 0);
    add_logf(server_log_filename, LOG_INFO, "Sent ping to client on socket: %d", current_client->socket);
  }
  return 0;
}

void* send_measurement_control_requests(void* arg) {
  while (!threads_done) {
    sleep(1);
    hashmap_iter(server.clients, (hashmap_callback) send_measurement_control_request, NULL);
  }
  return NULL;
}

int send_measurement_control_request(void *data, const char *key, void *value) {
  json_t *measurement_control_message;
  char *json_str_outgoing;
  size_t json_str_len;

  measurement_control_message = json_object();
  json_object_set(measurement_control_message, "message_type", json_integer(measurement_control_request));
  json_str_outgoing = json_dumps(measurement_control_message,0);
  json_str_len = strlen(json_str_outgoing);
  
  client_t* current_client = (client_t*) value;
  write(current_client->socket, &json_str_len, json_str_len);
  size_t written = write(current_client->socket, json_str_outgoing, json_str_len);

  assert(json_str_len == written);
  free(json_str_outgoing);

  //add_logf(server_log_filename, LOG_INFO, "Send measurement control request on socket: %d", current_client->socket);
  // ####################################################### 

  /* s_message measurement_control_message;
  memset(&measurement_control_message, 0, sizeof(measurement_control_message));
  measurement_control_message.message_type = measurement_control_request;
  
  
  return 0;*/
}
