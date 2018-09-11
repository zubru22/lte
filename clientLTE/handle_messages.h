#ifndef HANDLE_MESSAGES_H
#define HANDLE_MESSAGES_H

#include "../message.h"

#ifndef UNISTD_H
#include <unistd.h>
#endif

int receive_ping(int, s_message*);
int send_pong(int, s_message*);

#endif