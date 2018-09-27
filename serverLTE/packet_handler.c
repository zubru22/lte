#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

const int SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD = 3;

void handle_random_access_request(int client_socket, s_message message) {
  int16_t received_ra_rnti = message.message_value.message_preamble.ra_rnti;
  int new_client_phone_number = message.message_value.message_preamble.phone_number;
  add_logf(server_log_file, LOG_INFO, "New client's phone number: %d", new_client_phone_number);
  uint8_t preamble_index = extractPreambleIndex(received_ra_rnti);
  time_t current_timestamp = time(NULL);

  send_random_access_response(client_socket, preamble_index, current_timestamp);
  update_client_by_ra_rnti_data(client_socket, preamble_index, current_timestamp, received_ra_rnti, new_client_phone_number);
  add_logf(server_log_file, LOG_INFO, "Random Access response sent");
}

void handle_pong(int client_socket) {
  //update client's lst activity timestamp
  client_t* to_be_updated = get_client_by_socket(server.clients, client_socket);
  to_be_updated->last_activity = time(NULL);
  add_logf(server_log_file, LOG_INFO, "Received pong on socket: %d", client_socket);
}

void handle_client_power_off(int client_socket) {
  delete_client_from_hashmap(server.clients, client_socket);
}


void parse_packet(int number_of_event) {
  int client_socket = server.events[number_of_event].data.fd;

  //add_logf(server_log_file, LOG_INFO, "Parsing packet from socket: %d", client_socket);
  s_message message;
  memset(&message, 0, sizeof(message));
  int number_of_bytes_read = read(client_socket, &message, sizeof(message));
  if (number_of_bytes_read == -1) {
    error("read in parse_packet");
  } else if (number_of_bytes_read == 0) {
    add_logf(server_log_file, LOG_INFO, "Client disconnected: %d", client_socket);
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
    case x2ap_resource_status_request:
      handle_x2ap_resource_status_request(message.message_value.handover.client_socket);
      break;
    case x2ap_resource_status_response:
      handle_x2ap_resource_status_response(message.message_value.handover.client_socket);
      break;
    case x2ap_handover_request:
      handle_x2ap_handover_request(message.message_value.handover.client_socket);
      break;
    case x2ap_handover_request_acknowledge:
      handle_x2ap_handover_request_acknowledge(message.message_value.handover.client_socket);
      break;
    case SMS:
      forward_sms_message(message, client_socket);
      break;
    case resource_request:
      handle_resource_request(client_socket, message);
      break;
    case forwarded_message:
      handle_message_from_enb(message);
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
  if(send_thread_safe(socket, &response, sizeof(response), 0) == -1) {
    error("send in send_rrc_setup");
  }
  add_logf(server_log_file, LOG_INFO, "Sent RRC setup");
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
    add_logf(server_log_file, LOG_INFO, "Sent value: %d", preamble_index);
    send_thread_safe(socket, &response, sizeof(response), 0);
}

void handle_low_battery_request(int client_socket) {
  add_logf(server_log_file, LOG_INFO, "Battery LOW on client: %d", client_socket);
  client_t* client_with_low_battery = get_client_by_socket(server.clients, client_socket);
  client_with_low_battery->battery_state = LOW;
}

void handle_high_battery_request(int client_socket) {
  add_logf(server_log_file, LOG_INFO, "Battery HIGH on client: %d", client_socket);
  client_t* client_with_high_battery = get_client_by_socket(server.clients, client_socket);
  client_with_high_battery->battery_state = OK;
}

void* ping_and_timeout_in_thread(void* arg) {
  while (!threads_done) {
    sleep(1);
    hashmap_iter(server.clients, (hashmap_callback) ping_client, NULL);
    hashmap_iter(server.clients, (hashmap_callback) handle_client_inactivity, NULL);
  }
  return NULL;
}

int ping_client(void *data, const char *key, void *value) {
  time_t current_time = time(NULL);
  client_t* current_client = (client_t*) value;
  if(current_client->is_server) {
    return 0;
  }
  time_t time_since_last_activity = current_time - current_client->last_activity;
  bool should_ping = (current_client->battery_state == OK && (time_since_last_activity > PING_TIME_NORMAL))
  || (current_client->battery_state == LOW && (time_since_last_activity > PING_TIME_LOW_BATTERY));

  if (should_ping && current_client->is_server == false) {
    s_message ping_message;
    memset(&ping_message, 0, sizeof(ping_message));
    ping_message.message_type = ping;
    send_thread_safe(current_client->socket, &ping_message, sizeof(ping_message), 0);
    add_logf(server_log_file, LOG_INFO, "Sent ping to client on socket: %d", current_client->socket);
  }
  return 0;
}

void* send_measurement_control_requests(void* arg) {
  while (!threads_done) {
    interruptible_sleep(SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD);
    hashmap_iter(server.clients, (hashmap_callback) send_measurement_control_request, NULL);
  }
  return NULL;
}

int send_measurement_control_request(void *data, const char *key, void *value) {
  client_t* current_client = (client_t*) value;
  if(current_client->is_server) {
        return 0;
    }
  if (current_client->is_server == false) {
    s_message measurement_control_message;
    memset(&measurement_control_message, 0, sizeof(measurement_control_message));
    measurement_control_message.message_type = measurement_control_request;
    send_thread_safe(current_client->socket, &measurement_control_message, sizeof(measurement_control_message), 0);
    add_logf(server_log_file, LOG_INFO, "Send measurement control request on socket: %d", current_client->socket);
  }
  return 0;
}

void log_event(const char* event, int client_socket) {
  add_logf(server_log_file, LOG_INFO, "Received event %s on socket %d", event, client_socket);
}

void handle_measurement_report(int client_socket, s_message measurement_report_message) {
  char* event;
  switch (measurement_report_message.message_value.events) {
    case a1:
      log_event("A1", client_socket);
      break;
    case a2:
      log_event("A2", client_socket);
      break;
    case a3:
      log_event("A3", client_socket);
      handle_a3_event(client_socket);
      break;
    case a4:
      log_event("A4", client_socket);
      break;
    case a5:
      log_event("A5", client_socket);
      break;
    case def:
      log_event("DEF", client_socket);
      break;
  }

}

void handle_a3_event(int client_socket) {
  add_logf(server_log_file, LOG_INFO, "handle_a3_event with client socket: %d", client_socket);
  if (server.target_socket == -2) {
    connect_to_target_server();
  }
  // send x2ap resource status request
  s_message handover_x2ap_resource_status_request;
  memset(&handover_x2ap_resource_status_request, 0, sizeof(handover_x2ap_resource_status_request));
  handover_x2ap_resource_status_request.message_type = x2ap_resource_status_request;
  handover_x2ap_resource_status_request.message_value.handover.client_socket = client_socket;
  if (send_thread_safe(server.target_socket, &handover_x2ap_resource_status_request, sizeof(handover_x2ap_resource_status_request), 0) == -1) {
    error("send in handle_a3_event");
  }
  add_logf(server_log_file, LOG_INFO, "send handover_x2ap_resource_status_request to target at socket: %d", server.target_socket);
  handle_handover();
}

void handle_handover() {
  add_logf(server_log_file, LOG_INFO, "expecting handover messages at target server on socket: %d\n", server.target_socket);
  s_message handover_message;
  memset(&handover_message, 0, sizeof(handover_message));
  while(1) {
    if(recv(server.target_socket, &handover_message, sizeof(handover_message), 0) == -1) {
      error ("recv in handle_handover");
    }
    switch(handover_message.message_type) {
      case x2ap_resource_status_response:
        handle_x2ap_resource_status_response(handover_message.message_value.handover.client_socket);
        break;
      case x2ap_handover_request_acknowledge:
        handle_x2ap_handover_request_acknowledge(handover_message.message_value.handover.client_socket);
        return;
      default:
        break;
     }
     memset(&handover_message, 0, sizeof(handover_message));
   }
}

void handle_x2ap_resource_status_request(int client_socket) {
  add_logf(server_log_file, LOG_INFO, "received x2ap_resource_status_request from source server");
  server.target_socket = client_socket;
  client_t* source_server = get_client_by_socket(server.clients, client_socket);
  source_server->is_server = true;
  s_message handover_x2ap_resource_status_response;
  memset(&handover_x2ap_resource_status_response, 0, sizeof(handover_x2ap_resource_status_response));
  handover_x2ap_resource_status_response.message_type = x2ap_resource_status_response;
  handover_x2ap_resource_status_response.message_value.handover.client_socket = client_socket;
  //sleep(10);
  if (send_thread_safe(server.target_socket, &handover_x2ap_resource_status_response, sizeof(handover_x2ap_resource_status_response), 0) == -1) {
    error("send in handle_x2ap_resource_status_request");
  }
  add_logf(server_log_file, LOG_INFO, "send x2ap_resource_status_response to source server");
}

void handle_x2ap_resource_status_response(int client_socket) {
  add_logf(server_log_file, LOG_INFO, "received x2ap_resource_status_request from target server");
  s_message handover_x2ap_handover_request;
  memset(&handover_x2ap_handover_request, 0, sizeof(handover_x2ap_handover_request));
  handover_x2ap_handover_request.message_type = x2ap_handover_request;
  handover_x2ap_handover_request.message_value.handover.client_socket = client_socket;
  if (send_thread_safe(server.target_socket, &handover_x2ap_handover_request, sizeof(handover_x2ap_handover_request), 0) == -1) {
    error("send in handle_x2ap_resource_status_response");
  }
   add_logf(server_log_file, LOG_INFO, "send x2ap_handover_request to target server");
}

void handle_x2ap_handover_request(int client_socket) {
  add_logf(server_log_file, LOG_INFO, "received x2ap_handover_request from source server");
  s_message handover_request_acknowledge;
  memset(&handover_request_acknowledge, 0, sizeof(handover_request_acknowledge));
  handover_request_acknowledge.message_type = x2ap_handover_request_acknowledge;
  handover_request_acknowledge.message_value.handover.client_socket = client_socket;
  if (send_thread_safe(server.target_socket, &handover_request_acknowledge, sizeof(handover_request_acknowledge), 0) == -1) {
    error("send in handle_x2ap_resource_status_response");
  }
  add_logf(server_log_file, LOG_INFO, "send x2ap_handover_request_acknowledge to source server");
}

void handle_x2ap_handover_request_acknowledge(int client_socket) {
  add_logf(server_log_file, LOG_INFO, "received x2ap_handover_request_acknowledge from target server");
  s_message rrc_connection_reconfiguration_request_message;
  memset(&rrc_connection_reconfiguration_request_message, 0, sizeof(rrc_connection_reconfiguration_request_message));
  rrc_connection_reconfiguration_request_message.message_type = rrc_connection_reconfiguration_request;
  rrc_connection_reconfiguration_request_message.message_value.handover_request.port = server.target_port;
  memset(rrc_connection_reconfiguration_request_message.message_value.handover_request.ip_address, 0, sizeof(rrc_connection_reconfiguration_request_message.message_value.handover_request.ip_address));
  strcpy(rrc_connection_reconfiguration_request_message.message_value.handover_request.ip_address, "127.0.0.1");
  printf ("port = %d\n", rrc_connection_reconfiguration_request_message.message_value.handover_request.port);
  printf ("ip_address = %s\n", rrc_connection_reconfiguration_request_message.message_value.handover_request.ip_address);
  // here we need to get client with client_socket
  if (send_thread_safe(client_socket, &rrc_connection_reconfiguration_request_message, sizeof(rrc_connection_reconfiguration_request_message), 0) == -1) {
    error("handle_x2ap_handover_request_acknowledge");
  }
  add_logf(server_log_file, LOG_INFO, "sent rrc_connection_reconfiguration_request to client to handover");
}

void handle_resource_request(int client_socket, s_message resource_request) {
  bool is_file_present = false;

  // TODO
  //char* filename = resource_request.message_value.requested_file;
  char* filename = "obrazek.png";
  add_logf(server_log_file, LOG_INFO, "File to be sent: %s\n", filename);

  s_message file_request_response;
  file_request_response.message_type = resource_response;

  FILE* file_to_be_sent = fopen(filename, "rb");
  if (file_to_be_sent == NULL) {
    add_logf(server_log_file, LOG_ERROR, "Error opening file %s", filename);
    is_file_present = false;
  } else {    
    is_file_present = true;
  }

  file_request_response.message_value.resource_state = is_file_present;

  if (send_thread_safe(client_socket, &file_request_response, sizeof(file_request_response), 0) == -1) {
    error("Error sending resource status");
  } else {
    add_logf(server_log_file, LOG_SUCCESS, "Sent \"File OK\" notification");
  }

  if(!is_file_present) {
    return;
  }

  struct stat st;
  stat(filename, &st);
  __off_t file_size = st.st_size;

  add_logf(server_log_file, LOG_INFO, "Size of file to be sent (bytes): %lu", file_size);

  s_message data_message_tag;
  data_message_tag.message_type = data_start;
  memset(data_message_tag.message_value.buffer, 0, BUFFER_SIZE);
  memcpy(data_message_tag.message_value.buffer, filename, BUFFER_SIZE);
  if (send_thread_safe(client_socket, &data_message_tag, sizeof(data_message_tag), 0) == -1) {
    error("Error sending data start");
  } else {
    add_logf(server_log_file, LOG_SUCCESS, "START SEND DATA");
  }

  nanosleep((const struct timespec[]){{0, 100000L}}, NULL);
  s_message data_message;
  data_message.message_type = data;

  int bytes_read = 0;
  int bytes_sent = 0;
  int packets_sent = 0;
  while (bytes_read < file_size) {
    nanosleep((const struct timespec[]){{0, 100000L}}, NULL);

    fseek(file_to_be_sent, bytes_read, SEEK_SET);
    memset(data_message.message_value.buffer, 0, BUFFER_SIZE);
    fread(data_message.message_value.buffer, BUFFER_SIZE, 1, file_to_be_sent);

    bytes_read += BUFFER_SIZE;
    bytes_sent = send_thread_safe(client_socket, &data_message, sizeof(data_message), 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
      error("Error sending data");
    } else {
      add_logf(server_log_file, LOG_SUCCESS, "(SOCKET: %d) Bytes of file sent: %d", client_socket, bytes_sent);
      packets_sent++;
    }

  }

  data_message_tag.message_type = data_end;
  if (send_thread_safe(client_socket, &data_message_tag, sizeof(data_message_tag), 0) == -1) {
    error ("Error sending data start");
  } else {
    add_logf(server_log_file, LOG_SUCCESS, "File transfered! Packets sent: %d", packets_sent);
  }

}

void interruptible_sleep(int seconds) {
  for (int i = 0; i < seconds; i++) {
    sleep(1);
  }
}

void handle_message_from_enb(s_message passed_message) {
  char temp_receivers_phone_number[9];
  strncpy(temp_receivers_phone_number, passed_message.message_value.text_message, 9);
  int receivers_phone_number = atoi(temp_receivers_phone_number);
  int receivers_socket = get_clients_socket_by_MSIN(server.clients, receivers_phone_number);
  if(receivers_socket == 0) {
    add_logf(server_log_file, LOG_WARNING, "No client with number %d on this server", receivers_phone_number);
    return;
  }
  char wrapped_message_to_send[MESSAGE_LENGTH];
  strncpy(wrapped_message_to_send, passed_message.message_value.text_message+9, MESSAGE_LENGTH-9);
  passed_message.message_type = SMS;
  memset(passed_message.message_value.text_message, '\0', sizeof(char)*MESSAGE_LENGTH );
  memcpy(passed_message.message_value.text_message, wrapped_message_to_send, MESSAGE_LENGTH);

  if (send(receivers_socket, &passed_message, sizeof(passed_message), 0) == -1) {
    warning("Unable to pass message");
  } else {
    add_logf(server_log_file, LOG_SUCCESS, "Successfully passed SMS to client");
  }
}


