// This functionality is about to create Random Access Preamble (RA-RNTI) and wait for Random Access Response (C-RNTI) from eNB
#include "random_access.h"

// This function generates and returns ra_rnti for prach preamble and modifies original structure
void generate_ra_rnti(preamble* s_preamble) {
    const char max_t_id = 10;
    const char max_f_id = 6;
    s_preamble->ra_rnti = (1 + ( rand() % max_t_id) + 10 * (rand() % max_f_id) );
    
}

// This function sends prach preamble to eNodeB
int send_prach_preamble(int sockfd, s_message* message, void (*ra_rnti_generator_func)(preamble*)) {
    // First we need to fill preamble structure
    ra_rnti_generator_func(&message->message_value.message_preamble);
    //Then we need to set message type to RA_RNTI
    message->message_type = RA_RNTI;
    //Send struct
    if(-1 == write(sockfd, (s_message*)message, sizeof(message)))
        return -1;

    return 0;
}

// This function receives response from eNodeB
int receive_prach_response(int socketfd, s_message* message)
{
    if(-1 == read(socketfd, (s_message*)message, sizeof(*message)))
        return -1;
    return 0;
}