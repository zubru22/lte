#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

const int SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD = 3;

void handle_random_access_request(int client_socket, s_message message) {
  int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;
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

  add_logf(server_log_filename, LOG_INFO, "Parsing packet from socket: %d", client_socket);
  s_message message;
  memset(&message, 0, sizeof(message));
  int number_of_bytes_read = read(client_socket, &message, sizeof(message));
  if (number_of_bytes_read == -1) {
    error("read in parse_packet");
  } else if (number_of_bytes_read == 0) {
    add_logf(server_log_filename, LOG_INFO, "Client disconnected: %d", client_socket);
    close(client_socket);
    delete_client_from_hashmap(server.clients, client_socket);
    return;
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
      break;
    case pong:
      handle_pong(client_socket);
      break;
    case ue_off:
      handle_client_power_off(client_socket);
      break;
    case measurement_report:
      handle_measurement_report(client_socket, message);
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
  memset(&response, 0, sizeof(response));
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
}

int send_measurement_control_request(void *data, const char *key, void *value) {
  s_message measurement_control_message;
  memset(&measurement_control_message, 0, sizeof(measurement_control_message));
  measurement_control_message.message_type = measurement_control_request;
  client_t* current_client = (client_t*) value;
  send(current_client->socket, &measurement_control_message, sizeof(measurement_control_message), 0);
  add_logf(server_log_filename, LOG_INFO, "Send measurement control request on socket: %d", current_client->socket);
}

void handle_measurement_report(int client_socket, s_message measurement_report_message) {
  switch (measurement_report_message.message_value.events) {
    case a1:
      printf ("a1\n");
      break;
    case a2:
      printf ("a2\n");
      break;
    case a3:
      printf ("a3\n");
      handle_a3_event(client_socket);
      break;
    case a4:
      printf ("a4\n");
      break;
    case a5:
      printf ("a5\n");
    default:
      break;
  }
}

void handle_a3_event(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "handle_a3_event with client socket: %d", client_socket);
  if (server.target_socket == -2) {
    connect_to_target_server();
  }
  // send x2ap resource status request
  s_message handover_x2ap_resource_status_request;
  memset(&handover_x2ap_resource_status_request, 0, sizeof(handover_x2ap_resource_status_request));
  handover_x2ap_resource_status_request.type_of_message = x2ap_resource_status_request;
  handover_x2ap_resource_status_request.handover.client_socket = client_socket;
  if (send(server.target_socket, &handover_x2ap_resource_status_request, sizeof(handover_x2ap_resource_status_request), 0) == -1) {
    error("send in handle_a3_event");
  }
  add_logf(server_log_filename, LOG_INFO, "send handover_x2ap_resource_status_request to target at socket: %d", server.target_socket);
}

void handle_x2ap_resource_status_request(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "received x2ap_resource_status_request from source server");
  s_message handover_x2ap_resource_status_response;
  handover_x2ap_resource_status_response.type_of_message = x2ap_resource_status_response;
  handover_x2ap_resource_status_response.handover.client_socket = client_socket;
  memset(&handover_x2ap_resource_status_response, 0, sizeof(handover_x2ap_resource_status_response));
  if (send(server.target_socket, &handover_x2ap_resource_status_response, sizeof(handover_x2ap_resource_status_response), 0) == -1) {
    error("send in handle_a3_event");
  }
  add_logf(server_log_filename, LOG_INFO, "send x2ap_resource_status_response to source server");
}

void handle_x2ap_resource_status_response(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "received x2ap_resource_status_request from target server");
  s_message handover_x2ap_handover_request;
  handover_x2ap_handover_request.type_of_message = x2ap_handover_request;
  handover_x2ap_handover_request.handover.client_socket = client_socket;
  memset(&handover_x2ap_handover_request, 0, sizeof(handover_x2ap_handover_request));
  if (send(server.target_socket, &handover_x2ap_handover_request, sizeof(handover_x2ap_handover_request), 0) == -1) {
    error("send in handle_a3_event");
  }
  add_logf(server_log_filename, LOG_INFO, "send x2ap_handover_request to target server");
}

void handle_x2ap_handover_request(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "received x2ap_handover_request from source server");
  s_message handover_request_acknowledge;
  handover_request_acknowledge.type_of_message = x2ap_handover_request_acknowledge;
  handover_request_acknowledge.handover.client_socket = client_socket;
  memset(&handover_request_acknowledge, 0, sizeof(handover_request_acknowledge));
  if (send(server.target_socket, &handover_request_acknowledge, sizeof(handover_request_acknowledge), 0) == -1) {
    error("send in handle_a3_event");
  }
  add_logf(server_log_filename, LOG_INFO, "send x2ap_handover_request_acknowledge to source server");
}

void handle_x2ap_handover_request_acknowledge(int client_socket) {
  add_logf(server_log_filename, LOG_INFO, "received x2ap_handover_request_acknowledge from target server");
  s_message rrc_connection_reconfiguration_request_message;
  rrc_connection_reconfiguration_request_message.type_of_message = rrc_connection_reconfiguration_request;
  memset(&rrc_connection_reconfiguration_request_message, 0, sizeof(rrc_connection_reconfiguration_request_message));
  // here we need to get client with client_socket
  /*if (send(server.target_socket, &rrc_connection_reconfiguration_request_messagee, sizeof(rrc_connection_reconfiguration_request_message), 0) == -1) {
    error("send in handle_a3_event");
  }*/
}
