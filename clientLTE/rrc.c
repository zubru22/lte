/* This functionality is about sending RRC Connection Request to eNodeB, 
* waiting for RCC Connection Setup from RRC and finally sending back
* the RRC Connection Complete message */
#include "rrc.h"
#include "../logs/logs.h"

// This function generates ue_identity random key
void generate_ue_identity(json_t* ue_identity) {
    json_object_set(ue_identity, "mme_code", json_integer(rand() % (int)pow(2, 8)));
    json_object_set(ue_identity, "mtmsi", json_integer(rand() % (int)pow(2, 32)));
}

// This function sends rrc request from UE to eNodeB. Function returns -1 on error and 0 on success
int send_rrc_connection_request(int socketfd, json_t* json_message, void (*generate_ue_identity_func)(json_t*)) {
    char *json_str;
    size_t json_str_len;
    int number_of_bytes_read;

    json_object_set(json_message, "message_type", json_string("rrc_request"));
    generate_ue_identity_func(json_message);
    json_object_set(json_message, "establishment_cause", json_integer(MO_SIGNALING));
    json_str = json_dumps(json_message, 0);
    json_str_len = strlen(json_str);

    write(socketfd, &json_str_len, json_str_len);

    // Write json string length to the socket
    if(-1 == write(socketfd, &json_str_len, json_str_len))
        return -1;

    number_of_bytes_read = write(socketfd, json_str, json_str_len);

    if (number_of_bytes_read != json_str_len)
        return -1;

    return 0;
}

// This function sends rrc setup complete from UE to eNodeB. Function returns -1 on error and 0 on success
int send_rrc_setup_complete(int socketfd, s_message* message) {
    if (message->message_type == rrc_setup) {
        message->message_type = rrc_complete;
        message->message_value.message_complete.mcc = 234;
        message->message_value.message_complete.mnc = 15;

        if(-1 == write(socketfd, (s_message*)message, sizeof(*message)))
            return -1;
        return 0;
    }
    return -1;
}

/* This function receives response from eNodeB. It returns -1 if receiving message failed, returns 0 if received message is rrc_setup, if received message is not 
    rrc_setup type 0 is returned */
int receive_rrc_setup(int socketfd, s_message* received, s_message* message) {
   if(-1 == recv(socketfd, (s_message*) received, sizeof(*received), 0)) {
        add_logf(client_log_filename, LOG_ERROR, "Error on recv() rrc_setup.");
        return -1;
    }
    else
        add_logf(client_log_filename, LOG_SUCCESS, "Successfully received rrc setup!");

    //after properly receiving rrc setup we call func to sent rrc setup_complete
    if(send_rrc_setup_complete(socketfd, received) == -1) {
        add_logf(client_log_filename, LOG_ERROR, "Failed to send rrc setup!");
        return -1;
    }
    else
        add_logf(client_log_filename, LOG_SUCCESS, "Successfully send rcc setup complete!");
    return 0;
}
