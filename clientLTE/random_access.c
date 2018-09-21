// This functionality is about to create Random Access Preamble (RA-RNTI) and wait for Random Access Response (C-RNTI) from eNB
#include "random_access.h"
#ifndef STDBOOL_H
#include <stdbool.h>
#endif

// This function generates and returns ra_rnti for prach preamble and modifies original structure
int generate_ra_rnti(void) {
    const char max_t_id = 10;
    const char max_f_id = 6;
    return (1 + (rand() % max_t_id) + 10 * (rand() % max_f_id));
}

// This function sends prach preamble to eNodeB. Function returns -1 on error and 0 on success
int send_prach_preamble(int sockfd, json_t* message, int (*ra_rnti_generator_func)(void)) {
    char *json_message;

    // We need to set message type to random_access_request
    json_object_set(message, "message_type", json_integer(random_access_request));
    // We need to fill preamble value (ra_rnti)
    json_object_set(message, "ra_rnti", json_integer(ra_rnti_generator_func()));
    // Convert json object to string, 0 means no formating
    json_message = json_dumps(message, 0);
    // Length of json string to be send
    size_t json_message_len = strlen(json_message) + 1;
    
    // Printing for debugging
    printf("message size: %lu\n", json_message_len);
    
    // Send length of json string
    write(sockfd, &json_message_len, json_message_len);
    size_t written = write(sockfd, json_message, json_message_len);
    printf("written: %lu\n", written);
    json_dumpf(message, stdout, 0);
    
    free(json_message);
    return 0;
}

// This function receives response from eNodeB. Function returns -1 on error #1, 1 on error #2 and 0 on success
int receive_prach_response(int socketfd, s_message* received, s_message* message) {
    if(-1 == recv(socketfd, (s_message*) received, sizeof(*received), 0))
        return -1;

    if(random_access_response == received->message_type) {
        int8_t value_received = received->message_value.message_response.rapid;
        int8_t value_expected = (message->message_value.message_preamble.ra_rnti & 0b1100000000000000) >> 8;
        message->message_value.message_request.c_rnti = (value_expected & 0b11000000);
        if (value_received == value_expected)
            return 0;
    }
    return 1;
}