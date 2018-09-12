#ifndef HANDLE_MESSAGES_H
#define HANDLE_MESSAGES_H

#include "../message.h"

#ifndef UNISTD_H
#define UNISTD_H
#include <unistd.h>
#endif

int receive_ping(int, s_message*);
int send_pong(int, s_message*);
int send_ue_off_signal(int, s_message*);
int receive_signal_level_request(int, s_message*);
int send_signal_level_response(int, s_message*);

#endif