/* This funcionality is about simulating UE battery DRX cycle */
#ifndef USER_EQUIPMENT_H
#include "user_equipment.h"
#endif 

// This function initializes battery. Should only be used once.
void initialize_battery_life(ue_battery* battery) {
    battery->power_percentage = 100;
    battery->power_is_low = false;
    time(&battery->starting_time);
}
// This function updates battery state. Decrases battery power every 'battery_decrase_time' seconds in order
// to simulate background processes going on in UE. Function oughts to be used in program's main loop.
// On top of that, lfunction returns zero if battery goes dead and 1 if it is still alive.
int update_battery(ue_battery* battery) {
    if(battery->power_percentage <= 0)
        return 0;
    
    if(battery->power_percentage <= 20)
        battery->power_is_low = true;
    
    const static time_t battery_decrase_time = 15;
    time_t time_now;
    time(&time_now);
    if((time_now - battery->starting_time) >= battery_decrase_time) {
        battery->power_percentage -= 1;
        // Reset starting_time to current time
        time(&battery->starting_time);
    }
    return 1;
}

// This function decrases battery life by 'decrase_amount' every time ping comes in
void decrase_after_ping(ue_battery* battery) {
    const static int8_t decrase_amount = 2;
    
    battery->power_percentage -= decrase_amount;
}

// This function sends low battery notification to eNodeB in order to induce battery saving mode
int send_low_battery_notification(int socketfd, s_message* message) {
    message->message_type = ue_battery_low;
    message->message_value = NULL;

    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        return -1;
    return 0;
}

// This function returns true if battery power goes from low to high, otherwise false
bool detect_high_battery_state(ue_battery* battery) {
    return ((battery->power_is_low) && (20 <= battery->power_percentage));
}

// This function sends notification to eNodeB if battery state is high again. Function returns 0 if all goes well,
// -1 in case if sending a message was not possible. If battery didn't change its state from low to high - function returns 1.
int send_high_battery_notification(int socketfd, s_message* message) {
    if(detect_high_battery_state()) {
        message->message_type = ue_battery_high;
        message->message_value = NULL;

        if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
            return -1;
        return 0;
    }
    return 1;
}