#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <sys/types.h>
#include <sys/socket.h>

void update_client_by_ra_rnti_data(int socket, int8_t preamble_index, time_t current_timestamp, int16_t received_ra_rnti);
int16_t get_client_rnti(int socket);
int notify_client_of_shutdown(void *data, const char *key, void *value);

#endif
