#ifndef RANDOM_ACCESS_H
#define RANDOM_ACCESS_H

#ifndef MESSAGE_H
#include "../message.h"
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void generate_ra_rnti(preamble*);
int send_prach_preamble(int, s_message*, void (*ra_rnti_generator_func)(preamble*));

#endif