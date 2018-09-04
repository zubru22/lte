// This functionality is about to create Random Access Preamble (RA-RNTI) and wait for Random Access Response (C-RNTI) from eNB
#include "random_access.h"

// This function generates and returns ra_rnti for prach preamble and modifies original structure
void generate_ra_rnti(preamble* s_preamble) {
    const char max_t_id = 10;
    const char max_f_id = 6;
    s_preamble->ra_rnti = (1 + ( rand() % max_t_id) + 10 * (rand() % max_f_id) );
    
}

// This function sends prach preamble to eNodeB
int send_prach_preamble(int sockfd, s_message* s_message, void (*ra_rnti_generator_func)(preamble*)) {
    // First we need to fill preamble structure
    ra_rnti_generator_func(&s_message->message_value.message_preamble);
    //Then we need to set message type to RA_RNTI
    s_message->message_type = RA_RNTI;
    //Send struct
    if(-1 == write(sockfd, &s_message, sizeof(s_message)))
        return -1;

    return 0;
}