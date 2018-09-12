/**
    @file handle_messages.h
    @brief Functions for handling ping and low battery signal
*/
/**
    @fn receive_ping(int, s_message*)
    @brief This function receives ping message from eNodeB.
    @param socketfd is socketfd from which message is received
    @param message is message receieved from the socket
    
    Function returns -1 on error, 0 on success.
    If function receives a message, but message's type isn't "ping" - it returns 1.
*/
/**
    @fn send_pong(int, s_message*)
    @brief This function sends ping complete (answer) to eNodeB.
    @param socketfd is socketfd to which message will be sent
    @param message is message send to socket

    Function returns -1 on error, 0 on success.
*/

#ifndef HANDLE_MESSAGES_H
#define HANDLE_MESSAGES_H

#include "../message.h"

#ifndef UNISTD_H
#define UNISTD_H
#include <unistd.h>
#endif

int receive_ping(int, s_message*);
int send_pong(int, s_message*);
int send_ue_off_signal(int, s_message*);

#endif