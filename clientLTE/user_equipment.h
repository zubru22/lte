/**
    @file user_equipment.h
    @brief Module for simulating battery life in UE
*/
/**
    @fn void initialize_battery_life(ue_battery*)
    @brief This function initializes battery. Should only be used once.
    @param ue_battery is struct to hold all the parameters of ours battery
    
*/
/**
    @fn void check_battery_status(int socketfd, int step, s_message* message, ue_battery* battery)
    @brief This function checks if battery power is low or high. Depends if it's low or high it sends alert to eNodeB.
    @param socketfd is socketfd to which message will be sent
    @param step is telling us how much do we decrease battery power level
    @param message is message send to socket
    @param battery is our struct to check status for
     
    @return Function returns -1 on error, 0 on success.
*/
/**
    @fn int update_battery(int, s_message*, ue_battery*)
    @brief This function is responsible for decreasing battery level over time. It also calls check_battery_status() func.
    @param socketfd is socketfd to which message will be sent
    @param message is message send to socket
    @param battery is our struct to check status for
     
    @return Function returns 1 on success.
*/
/**
    @fn void decrease_after_ping(int socketfd, s_message* message, ue_battery* battery)
    @brief This function is responsible for decreasing battery level over time. It also calls check_battery_status() func.
    @param socketfd is socketfd to which message will be sent
    @param message is message send to socket
    @param battery is our struct to check status for
*/
/**
    @fn int send_low_battery_notification(int, s_message*)
    @brief This function sends notification to eNodeB when battery power goes from high to low.
    @param socketfd is socketfd to which message will be sent
    @param message is message send to socket
     
    @return Function returns -1 on error, 0 on success.
*/
/**
    @fn int send_high_battery_notification(int, s_message*)
    @brief This function sends notification to eNodeB when battery power goes from low to high.
    @param socketfd is socketfd to which message will be sent
    @param message is message send to socket
     
    @return Function returns -1 on error, 0 on success.
*/
#ifndef USER_EQUIPMENT_H
#define USER_EQUIPMENT_H
#include <stdio.h>

#ifndef STDBOOL_H
#define STDBOOL_H
#include <stdbool.h>
#endif

#ifndef TIME_H
#define TIME_H
#include <time.h>
#endif

#ifndef TYPES_H
#define TYPES_H
#include <sys/types.h>
#endif

#ifndef UNISTD_H
#include <unistd.h>
#endif

#ifndef MESSAGE_H
#include "../message.h"
#endif

typedef struct s_ue_battery {
    int8_t power_percentage;
    bool power_is_low;
    bool charging;
    time_t starting_time;
} ue_battery;

void initialize_battery_life(ue_battery*);
int update_battery(int, s_message*, ue_battery*);
void decrease_after_ping(int socketfd, s_message* message, ue_battery* battery);
int send_low_battery_notification(int, s_message*);
int send_high_battery_notification(int, s_message*);
void check_battery_status(int socketfd, int step, s_message* message, ue_battery* battery);

#endif