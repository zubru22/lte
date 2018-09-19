#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

const int SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD = 5;

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
    sleep(SEND_MEASUREMENT_CONTROL_REQUEST_PERIOD);
    hashmap_iter(server.clients, (hashmap_callback) send_measurement_control_request, NULL);
  }
  return NULL;
}

int send_measurement_control_request(void *data, const char *key, void *value) {
  s_message measurement_control_message;
  memset(&measurement_control_message, 0, sizeof(measurement_control_message));
  measurement_control_message.message_type = measurement_control_request;
  client_t* current_client = (client_t*) value;
  send(current_client->socket, &measurement_control_message, sizeof(measurement_control_message), 0);
  add_logf(server_log_filename, LOG_INFO, "Send measurement control request on socket: %d", current_client->socket);
  return 0;
}

int broadcast_sample(void *arg, const char *key, void *value) {
  client_t* current_client = (client_t*) value;

  char* filename = (char*) arg;
  add_logf(server_log_filename, LOG_INFO, "File to be sent: %s\n", filename);

  FILE* file_to_be_sent = fopen(filename, "rb");
  if (file_to_be_sent == NULL) {
    add_logf(server_log_filename, LOG_ERROR, "Error opening file %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  struct stat st;
  stat(filename, &st);
  __off_t real_size = st.st_size;
  
  fseek(file_to_be_sent, 0L, SEEK_END);
  long file_size = ftell(file_to_be_sent);
  fseek(file_to_be_sent, 0, SEEK_SET);
  
  add_logf(server_log_filename, LOG_INFO, "Size of file (bytes): %lu\nReal size of file: %lu\n", file_size, real_size);
  
  s_message data_message_tag;
  data_message_tag.message_type = data_start;
  if (send(current_client->socket, &data_message_tag, sizeof(data_message_tag), 0) == -1) {
    add_logf(server_log_filename, LOG_ERROR, "Error sending data start");
    exit(EXIT_FAILURE);
  } else {
    add_logf(server_log_filename, LOG_SUCCESS, "START SEND DATA");
  }

  sleep(1);
  s_message data_message;
  data_message.message_type = data;

  int bytes_read = 0;
  int bytes_sent = 0;
  int packets_sent = 0;
  while (bytes_read < file_size) {
    //sleep(1);
    // nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    nanosleep((const struct timespec[]){{0, 100000L}}, NULL);

    fseek(file_to_be_sent, bytes_read, SEEK_SET);
    memset(data_message.message_value.buffer, 0, BUFFER_SIZE);
    fread(data_message.message_value.buffer, BUFFER_SIZE, 1, file_to_be_sent);
    
    //data_message.message_value.buffer[BUFFER_SIZE-1] = '\0';
    bytes_read += BUFFER_SIZE;
    bytes_sent = send(current_client->socket, &data_message, sizeof(data_message), 0);

    if (bytes_sent == -1) {
      add_logf(server_log_filename, LOG_ERROR, "Error sending data");
      exit(EXIT_FAILURE);
    } else {
      //add_logf(server_log_filename, LOG_SUCCESS, "Data sent: %s\n", data_message.message_value.buffer);
      //add_logf(server_log_filename, LOG_INFO, "Bytes sent: %d", bytes_sent);
      packets_sent++;
    }
    
  }


  data_message_tag.message_type = data_end;
  if (send(current_client->socket, &data_message_tag, sizeof(data_message_tag), 0) == -1) {
    add_logf(server_log_filename, LOG_ERROR, "Error sending data start");
    exit(EXIT_FAILURE);
  } else {
    add_logf(server_log_filename, LOG_SUCCESS, "File transfered! Packets sent: %d", packets_sent);
  }

  return 0;
}


// filename as arg
void* transfer_data(void* arg) {
  sleep(10);
  while (!threads_done) {
    hashmap_iter(server.clients, (hashmap_callback) broadcast_sample, arg);
    // not too fast, so that we see what is going on
    sleep(5);
    exit(0);
  }
  return NULL;
}
