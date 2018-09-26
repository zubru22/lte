#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <sys/types.h>
#include <sys/socket.h>

#ifndef CONFIG_H
#include "config.h"
#endif

/**
 * // TODO something's not right here, review this comment and change
    @brief Gets client by its socket and updates it data with data given as arguments
    @param[in] socket Socket of client's struct that should be updated
    @param[in] preamble_index Preamble index (sometimes called RAPID) of updated client
    @param[in] current_timestamp Updated timestamp, used as time of last activity and first connection
    @param[in] received_ra_rnti Updated RA RNTI, before this function it's usually not set.
*/
void update_client_by_ra_rnti_data(int socket, int8_t preamble_index, time_t current_timestamp, int16_t received_ra_rnti, int new_phone_number);
int16_t get_client_rnti(int socket);
/**
    @brief Invoked by hashmap_iter on every map element. Sends message to every UE about eNodeB shutting down.
    @param[in] *data Passed to function by hashmap iter if any data needs to be used inside, unused here.
    @param[in] *key Key of currently iterated client.
    @param[in] *key Value of currently iterated client.
    @return 0 required by hashmap library (1 would stop iterating).
*/
int notify_client_of_shutdown(void *data, const char *key, void *value);
/**
    @brief Invoked by hashmap_iter on every map element. Sends message to every UE about eNodeB shutting down.
    @param[in] *data Passed to function by hashmap iter if any data needs to be used inside, unused here.
    @param[in] *key Key of currently iterated client.
    @param[in] *key Value of currently iterated client.
    @return 0 required by hashmap library (1 would stop iterating).
*/
int handle_client_inactivity(void *data, const char *key, void *value);
/**
    @brief Thread safe wrapper method to send(), for sending message to client
    @see send() from <sys/socket.h>
    @param[in] client_socket Socket of client to be used for send
    @param[in] buf Buffer sent to client
    @param[in] size_of_buffer Size of buffer that is sent to user
    @param[in] flags Passed as-is to send function
    @return Number of bytes successfully sent
*/
ssize_t send_thread_safe(int client_socket, const void *buf, size_t size_of_buffer, int flags);

#endif
