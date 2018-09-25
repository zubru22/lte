/* This functionality is about handling default client server communication like pings and stuff */
#include "handle_messages.h"

extern FILE* log_file;
int bytes_received = 0;

// This function receives ping message from eNodeB. Function returns -1 on error, 0 on success.
// If function receives a message, but message's type isn't "ping" - it returns 1.
int receive_ping(int socketfd, s_message* message) {
    
    if(ping == message->message_type)
        return 0;
    
    return 1;
}

//This function sends ping complete (answer) to eNodeB. Function returns -1 on error, 0 on success.
int send_pong(int socketfd, s_message* message) { 
    message->message_type = pong;
    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        return -1;

    return 0;
}

// This function sends UE turn off message to eNodeB
int send_ue_off_signal(int socketfd, s_message* message) {
    message->message_type = ue_off;

    if(-1 == write(socketfd, (s_message*)message, sizeof(*message)))
        return -1;
    return 0;
}

bool receive_measurement_control_request(int socketfd, s_message* message) {
    assert(message != NULL);
    
    add_logf(log_file, LOG_SUCCESS, "Successfuly received Measurement Control request.");
    return true;    
}

void send_measurement_report(int socketfd, s_message* message, s_cells* cells) {
    assert(message != NULL);
    message->message_type = measurement_report;
    message->message_value.events = cells->current_event;
    
    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        add_logf(log_file, LOG_ERROR, "Failed to send Measurement Report!");
    else
        add_logf(log_file, LOG_SUCCESS, "Successfuly sent Measurement Report!");
}

void download_data(int socketfd, s_message* message, FILE* fp) {
    assert(message != NULL);

    fwrite(message->message_value.buffer,BUFFER_SIZE,1,fp);

    bytes_received += BUFFER_SIZE;
}

int send_resource_request(int socketfd, s_message* message) {
    message->message_type = resource_request;
    
    if(-1 == write(socketfd, (s_message*)message, sizeof(*message)))
        return -1;

    return 0;
}

int send_SMS(int socketfd, s_message* message, char* text_message) {
    message->message_type = SMS;
    strcpy(message->message_value.text_message,text_message);

    if(-1 == write(socketfd, (s_message*)message, sizeof(*message)))
        return -1;

    return 0;
}
