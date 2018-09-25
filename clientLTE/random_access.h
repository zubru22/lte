#ifndef RANDOM_ACCESS_H
#define RANDOM_ACCESS_H

#ifndef MESSAGE_H
#include "../message.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifndef UNISTD_H
#include <unistd.h>
#endif

void generate_ra_rnti(preamble*);
int generate_phone_number();
int send_prach_preamble(int, s_message*, void (*ra_rnti_generator_func)(preamble*));
int receive_prach_response(int, s_message*, s_message*);

#endif