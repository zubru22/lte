/**
    @file handle_messages.h
    @brief Module for handling ping and low battery signal
*/

#ifndef HANDLE_MESSAGES_H
#define HANDLE_MESSAGES_H

#include "../message.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "../logs/logs.h"
#include <assert.h>
#include <stdbool.h>
#include "user_equipment.h"
#include <errno.h>

#ifndef UNISTD_H
#define UNISTD_H
#include <unistd.h>
#endif

/**
    @brief This function receives ping message from eNodeB.
    @param[in] socketfd is socket from which message is received
    @param[in] message is message receieved from the socket
    @return Function returns -1 on error, 0 on success. If function receives a message, but message's type isn't "ping" - it returns 1.
*/
int receive_ping(int, s_message*);
/**
    @brief This function sends ping complete (answer) to eNodeB.
    @param[in] socketfd is socket to which message is send
    @param[out] message is message send to socket
    @return Function returns -1 on error, 0 on success.
*/
int send_pong(int, s_message*);
/**
    @brief This function sends UE off signal to eNodeB.
    @param[in] socketfd is socket to which message is send.
    @param[out] message* is a pointer to message which is send to socket
    @return Function returns -1 on error, 0 on success.
*/
int send_ue_off_signal(int, s_message*);
/**
    @brief This function receives Measurement Control request from eNodeB and calls sending function
    @param[in] socketfd is socket from which message is received.
    @param[in] message* is a pointer to message which is received.
    @return Function returns true if request was receieved and false if it failed to receive request.
*/
bool receive_measurement_control_request(int, s_message*);
void send_measurement_report(int, s_message*, s_cells*);
bool download_data(int, s_message*, FILE*);

#endif