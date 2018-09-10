/* This functionality is about handling default client server communication like pings and stuff */

// This function receives ping message from eNodeB. Function returns -1 on error, 0 on success.
int receive_ping(int socketfd, s_message* message) {
    if(-1 == read(socketfd, (s_message*)message, sizeof(*message))
        return -1;
        
    return 0;
}

//This function sends ping complete (answer) to eNodeB. Function returns -1 on error, 0 on success.
int send_pong(int socketfd, s_message* message) { 
    message->message_type = pong;
    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        return -1;

    return 0;
}