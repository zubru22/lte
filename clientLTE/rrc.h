/**
    @file rrc.h
    @brief Module for handling RRC step of communication between UE and eNodeB.

    This functionality is about sending RRC Connection Request to eNodeB, 
    waiting for RCC Connection Setup from RRC and finally sending back
    the RRC Connection Complete message.
*/
/**
    @fn void generate_ue_identity(s_stmsi*)
    @brief Generates user equipment id which consists of MME code and S-TMSI code.

    Both codes are generated randomly.
    @param[in] s_tmsi* is pointer to stmsi struct which is member of rrc_req struct.
*/
/**
    @fn send_rrc_connection_request(int, s_message*, void (*generate_ue_identity_func)(s_stmsi*));
    @brief This function sends RRC request from UE to eNodeB.

    Function returns -1 on error and 0 on success.
    @param[in] socketfd is socket descriptor to which request will be send.
    @param[in] message is a pointer to message, which will be send to eNodeB.
    @param[in] generate_ue_identity_func is a pointer to function which genereate UE identity.
    @return returns 0 on success and -1 when error occured.
*/
/**
    @fn send_rrc_setup_complete(int socketfd, s_message* message)
    @brief This function sends rrc setup complete from UE to eNodeB.

    Function returns -1 on error and 0 on success.
    @param[in] sockefd is socket to which message will be send.
    @param[in] message* is a pointer to message which will be send to eNodeB.
    @return returns 0 on success and -1 when error occured.
*/
/**
    @fn int receive_rrc_setup(int socketfd, s_message* received, s_message* message)
    @brief This function receives response from eNodeB.

    It returns -1 if receiving message failed, returns 0 if received message is rrc_setup, if received message is not 
    rrc_setup type 0 is returned.
    @param[in] socketfd is a socket from which message is received.
    @param[in] received* is a pointer to message which is received.
    @param[in] message* is a pointer to message which is send to eNodeB
    @return It returns -1 if receiving message failed, returns 0 if received message is rrc_setup, if received message is not 
    rrc_setup type 0 is returned
*/


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef RRC_H
#define RRC_H

#ifndef MESSAGE_H
#include "../message.h"
#endif

#include <sys/socket.h>
#include <unistd.h>

void generate_ue_identity(s_stmsi*);
int send_rrc_connection_request(int, s_message*, void (*generate_ue_identity_func)(s_stmsi*));
int send_rrc_setup_complete(int socketfd, s_message* message);
int receive_rrc_setup(int socketfd, s_message* received, s_message* message);

#endif