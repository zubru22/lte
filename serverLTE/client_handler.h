#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <sys/types.h>
#include <sys/socket.h>

/**
    @brief Gets client by its socket and updates it data with data given as arguments
    @param[in] socket Socket of client's struct that should be updated
    @param[in] preamble_index New preamble index (sometimes called RAPID) to be set
    @param[in] current_timestamp Updated timestamp, used as time of last activity and first connection
    @param[in] received_ra_rnti Updated RA RNTI, before this function it's usually not set.
*/
void update_client_by_ra_rnti_data(int socket, int8_t preamble_index, time_t current_timestamp, int16_t received_ra_rnti);
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

#endif
