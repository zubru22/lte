/* This funcionality is about simulating UE battery DRX cycle
* also it takes care of signal power measurement simulation*/
#ifndef USER_EQUIPMENT_H
#include "user_equipment.h"
#endif
#ifndef LOGS_H
#include "../logs/logs.h"
#endif
#include <assert.h>

// This function initializes battery. Should only be used once.
void initialize_battery_life(ue_battery* battery) {
    assert(battery != NULL);

    battery->power_percentage = 100;
    battery->power_is_low = false;
    battery->charging = false;
    time(&battery->starting_time);
}
void check_battery_status(int socketfd, int step, s_message* message, ue_battery* battery) {
    const int low_battery_threshold = 20;
    assert((message != NULL) && (battery != NULL));
    if(battery->power_percentage <= low_battery_threshold && (low_battery_threshold-step) < battery->power_percentage && !battery->charging) {
        battery->power_is_low = true;
        send_low_battery_notification(socketfd, message);
        add_logf(client_log_filename, LOG_INFO, "Send low bettery note!");
    }
    else if (battery->power_percentage >= low_battery_threshold && (low_battery_threshold+step) > battery->power_percentage && battery->charging) {
        battery->power_is_low = false;
        send_high_battery_notification(socketfd, message);
        add_logf(client_log_filename, LOG_INFO, "Send high bettery note!");
    }
}

// This function updates battery state. Decrases battery power every 'battery_decrase_time' seconds in order
// to simulate background processes going on in UE. Function oughts to be used in program's main loop.
// On top of that, lfunction returns zero if battery goes dead and 1 if it is still alive.
int update_battery(int socketfd, s_message* message, ue_battery* battery) {
    assert((message != NULL) && (battery != NULL));

    const int step = 12;

    check_battery_status(socketfd, step, message, battery);

    const static time_t battery_decrease_time = 1;
    time_t time_now;
    time(&time_now);
    if((time_now - battery->starting_time) >= battery_decrease_time) {
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
    assert((message != NULL) && (battery != NULL));

    static const int8_t decrase_amount = 2;

    check_battery_status(socketfd, decrase_amount, message, battery);

    if (battery->power_percentage - decrase_amount < 0)
        battery->power_percentage = 0;
    else if((battery->power_percentage != 0) && ((battery->power_percentage - decrase_amount) >= 0))
        battery->power_percentage -= decrase_amount;

}

// This function sends low battery notification to eNodeB in order to induce battery saving mode
int send_low_battery_notification(int socketfd, s_message* message) {
    assert(message != NULL);

    message->message_type = ue_battery_low;

    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        return -1;
    return 0;
}
// This function sends notification to eNodeB if battery state is high again. Function returns 0 if all goes well,
// -1 in case if sending a message was not possible.
int send_high_battery_notification(int socketfd, s_message* message) {
    assert(message != NULL);

    message->message_type = ue_battery_high;

    if(-1 == write(socketfd, (s_message*) message, sizeof(*message)))
        return -1;
    return 0;
}
// This function initializes cells, returns false if fails, otherwise true
void initialize_cells(s_cells* cells) {
    assert(cells != NULL);

    const int8_t threshold1 = 19; // thresholds are hardcoded to 20% and 80% of rsrp
    const int8_t threshold2 = 77;
    const int8_t max_rsrp = 97;
    const int8_t signal_course_value = 5;

    for(int8_t i = 0; i < NUMBER_OF_CELLS; i++) {
        cells->cells_signals[i].rsrp = rand() % max_rsrp;
        cells->cells_signals[i].thresholds[0] = threshold1;
        cells->cells_signals[i].thresholds[1] = threshold2;
        cells->current_event = def;
        cells->cells_signals[i].signal_course = signal_course_value;
    }

    time(&cells->starting_time);
}
// This function simulates change of signal power in each cell
void update_rsrps(s_cells* cells) {
    assert(cells != NULL);

    static const int8_t time_up = 1; // Variables like this are static, because they are used in function being called very often
    time_t time_now;
    const int8_t max_rsrp = 97;

    time(&time_now);

    if((time_now - cells->starting_time) > time_up) {
        for(int8_t i = 0; i < NUMBER_OF_CELLS; i++) {
            cells->cells_signals[i].rsrp += cells->cells_signals[i].signal_course;

            if(cells->cells_signals[i].rsrp > max_rsrp)
                cells->cells_signals[i].rsrp = max_rsrp;

            if(cells->cells_signals[i].rsrp < 0)
                cells->cells_signals[i].rsrp = 0;

            if((cells->cells_signals[i].rsrp >= max_rsrp) || (cells->cells_signals[i].rsrp <= 0)) {
                cells->cells_signals[i].signal_course *= -1; // Change sign of course (velocity) of signal's power change
            }
            printf("Signal power (%d): %d\n", i+1, cells->cells_signals[i].rsrp);
        }

        time(&cells->starting_time); // Actualise starting time
    }
}
// This function checks if there is one of events A1 or A2
s_event check_events(s_cells* cells) {
    static int8_t event_change = 5; // This variable is here only for event A1 and A2
    static bool a3_checked = false;
    static bool a4_checked = false;

    // Event change checks
    if(cells->cells_signals[0].rsrp < cells->cells_signals[0].thresholds[0] && 5 == event_change)
        event_change = 1;

    if(cells->cells_signals[0].rsrp > cells->cells_signals[0].thresholds[1] && 5 == event_change)
        event_change = 2;

    // Events A1, A2
    if(cells->cells_signals[0].rsrp > cells->cells_signals[0].thresholds[0] && 1 == event_change) {
        event_change = 5;
        return a1;
    }

    if(cells->cells_signals[0].rsrp < cells->cells_signals[0].thresholds[1] && 2 == event_change) {
        event_change = 5;
        return a2;
    }
    // Event A3
    if(cells->cells_signals[1].rsrp < cells->cells_signals[0].rsrp)
        a3_checked = false;

    if(cells->cells_signals[1].rsrp > cells->cells_signals[0].rsrp && false == a3_checked) {
        event_change = 5;
        a3_checked = true;
        return a3;
    }
    // Event A4
    if(cells->cells_signals[1].rsrp < cells->cells_signals[1].thresholds[1])
        a4_checked = false;

    if(cells->cells_signals[1].rsrp > cells->cells_signals[1].thresholds[1] && false == a4_checked) {
        event_change = 5;
        a4_checked = true;
        return a4;
    }

    //Default event - it actualy means no event whatsoever
    return def;
}
// This function sets signal events
void set_current_signal_event(s_cells* cells) {
    assert(cells != NULL);

    update_rsrps(cells);
    cells->current_event = check_events(cells);
}
