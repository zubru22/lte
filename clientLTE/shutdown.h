/**
    @file shutdown.h
    @brief Module for handling shutdown signal received from eNodeB
*/
/**
    @fn bool check_for_shutdown(int socketfd, s_message* message)
    @brief Checks for enb_off type of message and closes socket if shutdown message is received 
    @param[in] socketfd describes socket to which client is connected
    @param[in] message is sent between UE and eNodeB
    @return bool is returned, true when shutdown signal was received and false if it was not received
*/
#ifndef SHUTDOWN_H
#define SHUTDOWN_H
#include <stdbool.h>
#include <unistd.h>
#include "../message.h"
#include "../logs/logs.h"

bool check_for_shutdown(int socketfd, s_message* message);
#endif