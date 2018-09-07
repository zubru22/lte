#ifndef USER_EQUIPMENT_H
#define USER_EQUIPMENT_H

#ifndef STDBOOL_H
#define STDBOOL_H
#include <stdbool.h>
#endif

#ifndef TIME_H
#define TIME_H
#include <time.h>
#endif

#include <sys/types.h>
#include <unistd.h>

#ifndef MESSAGE_H
#include "../message.h"
#endif

typedef struct s_ue_battery {
    int8_t power_percentage;
    bool power_is_low;
    time_t starting_time;
} ue_battery;

void initialize_battery_life(ue_battery*);
int update_battery(ue_battery*);
void decrase_after_ping(ue_battery*);
int send_low_battery_notification(int, s_message*);
bool detect_high_battery_state(ue_battery*);
int send_high_battery_notification(int, s_message*, ue_battery*);

#endif