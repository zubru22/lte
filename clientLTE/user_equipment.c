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
// On top of that, function returns zero if battery goes dead and 1 if it is still alive.
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