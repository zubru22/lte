/* This functionality is about handling default client server communication like pings and stuff */
#include "handle_messages.h"

int bytes_received = 0;

// This function receives ping message from eNodeB. Function returns -1 on error, 0 on success.
// If function receives a message, but message's type isn't "ping" - it returns 1.
int receive_ping(int socketfd, s_message* message) {
    if(-1 == recv(socketfd, (s_message*)message, sizeof(*message), MSG_DONTWAIT))
        return -1;
    
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

    if(-1 == recv(socketfd, (s_message*)message, sizeof(*message), MSG_DONTWAIT))
        return false;
    
    if(message->message_type == measurement_control_request) {
        add_logf(client_log_filename, LOG_SUCCESS, "Successfuly received Measurement Control request.");
        return true;
    }

    add_logf(client_log_filename, LOG_WARNING, "Message type is not Measurement Control request!");
    return false;
}

void send_measurement_report(int socketfd, s_message* message, s_cells* cells) {
    assert(message != NULL);
    message->message_type = measurement_report;
    message->message_value.events = cells->current_event;
    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        add_logf(client_log_filename, LOG_ERROR, "Failed to send Measurement Report!");
    else
        add_logf(client_log_filename, LOG_ERROR, "Successfuly sent Measurement Report!");
}

bool download_data(int socketfd, s_message* message, FILE* fp) {
    assert(message != NULL);

    if(-1 == recv(socketfd, (s_message*)message, sizeof(*message), MSG_DONTWAIT))
        return false;
    
    if(message->message_type == data_start) {
        add_logf(client_log_filename, LOG_SUCCESS, "\n\nSuccessfully started downloading data!\n\n");
        fp = fopen("received_file","w+");
        return true;
    }

    else if(message->message_type == data) {
        add_logf(client_log_filename, LOG_SUCCESS, "Received data! %s", message->message_value.buffer);
        
        // TODO - don't open and close the file every time, do it on data_start and data_end
        
        FILE* received_file = fopen("received","ab+");
        fprintf(received_file, "%s", message->message_value.buffer);
        //fprintf(fp, "%s", message->message_value.buffer);
        bytes_received += BUFFER_SIZE-1;
        add_logf(client_log_filename, LOG_INFO, "Number of currently read bytes: %d", bytes_received);
        fclose(received_file);
        memset(message->message_value.buffer, 0, BUFFER_SIZE);
        return true;
    }

    else if(message->message_type == data_end) {
        fclose(fp);
        add_logf(client_log_filename, LOG_SUCCESS, "Successfully downloaded data!");
        return true;
    }
    return false;
}