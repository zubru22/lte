#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../message.h"
#include "server_init.h"

void parse_packet(int number_of_event);
void send_CRNTI(int socket, s_message message);
#endif