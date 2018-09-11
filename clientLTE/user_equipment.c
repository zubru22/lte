/* This funcionality is about simulating UE battery DRX cycle */
#ifndef USER_EQUIPMENT_H
#include "user_equipment.h"
#endif 

// This function initializes battery. Should only be used once.
void initialize_battery_life(ue_battery* battery) {
    battery->power_percentage = 100;
    battery->power_is_low = false;
    battery->charging = false;
    time(&battery->starting_time);
}
void check_battery_status(int socketfd, int step, s_message* message, ue_battery* battery) {
    if(battery->power_percentage <= 20 && (20-step) < battery->power_percentage && !battery->charging) {
        battery->power_is_low = true;
        send_low_battery_notification(socketfd, message);
        printf("Send low bettery note!\n");
    }
    else if (battery->power_percentage >= 20 && (20+step) > battery->power_percentage && battery->charging) {
        battery->power_is_low = false;
        send_high_battery_notification(socketfd, message);
        printf("Send high bettery note!\n");
    }
}

// This function updates battery state. Decrases battery power every 'battery_decrase_time' seconds in order
// to simulate background processes going on in UE. Function oughts to be used in program's main loop.
// On top of that, lfunction returns zero if battery goes dead and 1 if it is still alive.
int update_battery(int socketfd, s_message* message, ue_battery* battery) {
    const int step = 12;
    
    check_battery_status(socketfd, step, message, battery);

    const static time_t battery_decrase_time = 1;
    time_t time_now;
    time(&time_now);
    if((time_now - battery->starting_time) >= battery_decrase_time) {
        if (battery->charging == false)
            battery->power_percentage -= step;
        else
            battery->power_percentage += step;
        // Reset starting_time to current time
        time(&battery->starting_time);
    }
    
    if(battery->power_percentage <= 0) {
        battery->power_percentage = 0;
        battery->charging = true;
    }
    else if(battery->power_percentage >= 100) {
        battery->power_percentage = 100;
        battery->charging = false;
    }

    return 1;
}

// This function decrases battery life by 'decrase_amount' every time ping comes in
void decrease_after_ping(int socketfd, s_message* message, ue_battery* battery) {
    const static int8_t decrase_amount = 2;
    
    check_battery_status(socketfd, decrase_amount, message, battery);

    if (battery->power_percentage - decrase_amount < 0)
        battery->power_percentage = 0;
    else if((battery->power_percentage != 0) && ((battery->power_percentage - decrase_amount) >= 0))
        battery->power_percentage -= decrase_amount;

}

// This function sends low battery notification to eNodeB in order to induce battery saving mode
int send_low_battery_notification(int socketfd, s_message* message) {
    message->message_type = ue_battery_low;

    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        return -1;
    return 0;
}
// This function sends notification to eNodeB if battery state is high again. Function returns 0 if all goes well,
// -1 in case if sending a message was not possible.
int send_high_battery_notification(int socketfd, s_message* message) {
    message->message_type = ue_battery_high;

    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        return -1;
    return 0;
}