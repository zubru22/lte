// This functionality is about to create Random Access Preamble (RA-RNTI) and wait for Random Access Response (C-RNTI) from eNB
#include "random_access.h"
#include <stdbool.h>

// This function generates and returns ra_rnti for prach preamble and modifies original structure
void generate_ra_rnti(preamble* s_preamble) {
    const char max_t_id = 10;
    const char max_f_id = 6;
    s_preamble->ra_rnti = (1 + ( rand() % max_t_id) + 10 * (rand() % max_f_id) );
}

// This function sends prach preamble to eNodeB. Function returns -1 on error and 0 on success
int send_prach_preamble(int sockfd, s_message* message, void (*ra_rnti_generator_func)(preamble*)) {
    // First we need to fill preamble structure
    ra_rnti_generator_func(&message->message_value.message_preamble);
    //Then we need to set message type to random_access_request
    message->message_type = random_access_request;
    //Send struct
    if(-1 == write(sockfd, (s_message*)message, sizeof(*message)))
        return -1;

    return 0;
}

// This function receives response from eNodeB. Function returns -1 on error #1, 1 on error #2 and 0 on success
int receive_prach_response(int socketfd, s_message* received, s_message* message) {
    if(-1 == recv(socketfd, (s_message*) received, sizeof(*received), 0))
        return -1;

    if(random_access_response == received->message_type) {
        int8_t value_received = received->message_value.message_response.rapid;
        int8_t value_expected = (message->message_value.message_preamble.ra_rnti & 0b1100000000000000) >> 8;

        if (value_received == value_expected)
            return 0;
    }
    return 1;
}