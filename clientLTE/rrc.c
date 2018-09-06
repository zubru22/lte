/* This functionality is about sending RRC Connection Request to eNodeB, 
* waiting for RCC Connection Setup from RRC and finally sending back
* the RRC Connection Complete message */
#include "rrc.h"

// This function generates ue_identity random key
void generate_ue_identity(s_stmsi* ue_identity) {
    ue_identity->mme_code = rand() % (int)pow(2, 8);
    ue_identity->mtmsi = rand() % (int)pow(2, 32);
}

// This function sends rrc request from UE to eNodeB. Function returns -1 on error and 0 on success
int send_rrc_connection_request(int socketfd, s_message* message, void (*generate_ue_identity_func)(s_stmsi*)) {
    message->message_type = rrc_request;
    generate_ue_identity_func(&message->message_value.message_request.ue_identity);
    message->message_value.message_request.establishment_cause = MO_SIGNALING;

    if(-1 == write(socketfd, (s_message*)message, sizeof(*message)))
        return -1;
    return 0;
}

// This function sends rrc setup complete from UE to eNodeB. Function returns -1 on error and 0 on success
int send_rrc_setup_complete(int socketfd, s_message* message) {
    message->message_type = rrc_complete;
    message->message_value.message_complete.mcc = 234;
    message->message_value.message_complete.mnc = 15;

    if(-1 == write(socketfd, (s_message*)message, sizeof(*message)))
        return -1;
    return 0;
}