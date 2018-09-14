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

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#define NUMBER_OF_CELLS 2

typedef struct s_ue_battery {
    int8_t power_percentage;
    bool power_is_low;
    bool charging;
    time_t starting_time;
} ue_battery;

typedef struct _s_cell_signal {
    int8_t rsrp;
    int8_t thresholds[NUMBER_OF_CELLS];
    int8_t signal_course;
} s_cell_signal; 

typedef struct _s_cells {
    s_cell_signal cells_signals[NUMBER_OF_CELLS];
    time_t starting_time;
    s_event current_event;
} s_cells;

void initialize_battery_life(ue_battery*);
int update_battery(int, s_message*, ue_battery*);
void decrease_after_ping(int socketfd, s_message* message, ue_battery* battery);
int send_low_battery_notification(int, s_message*);
int send_high_battery_notification(int, s_message*);
void check_battery_status(int socketfd, int step, s_message* message, ue_battery* battery);
void initialize_cells(s_cells*);
void set_current_signal_event(s_cells*); 

#endif