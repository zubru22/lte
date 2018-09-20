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

#ifndef JANSSON_H
#include <string.h>
#include <jansson.h>
#endif

int generate_ra_rnti(void);
int send_prach_preamble(int, json_t*, int (*ra_rnti_generator_func)(void));
int receive_prach_response(int, s_message*, s_message*);

#endif