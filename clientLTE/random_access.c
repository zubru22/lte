// This functionality is about to create Random Access Preamble (RA-RNTI) and wait for Random Access Response (C-RNTI) from eNB
#include "random_access.h"

// This function generates and returns ra_rnti for prach preamble and modifies original structure
void generate_ra_rnti(preamble* s_preamble)
{
    const char max_t_id = 10;
    const char max_f_id = 6;

    s_preamble->ra_rnti = (1 + ( rand() % max_t_id) + 10 * (rand() % max_f_id) );
}

// This function sends prach preamble to eNodeB
int send_prach_preamble(int sockfd, s_message* s_message, void (*ra_rnti_generator_func)(preamble*))
{
    // First we need to fill preamble structure
    ra_rnti_generator_func(&s_message->message_value.message_preamble);
    s_message->message_type = RA_RNTI;

    if(-1 == send(sockfd, (int16_t*) &s_message->message_value.message_preamble, sizeof(int16_t), 0))
        return -1;

    return 0;
}