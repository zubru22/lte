#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef RRC_H
#define RRC_H

#ifndef MESSAGE_H
#include "../message.h"
#endif

#include <sys/socket.h>
#include <unistd.h>

void generate_ue_identity(s_stmsi*);
int send_rrc_connection_request(int, s_message*, void (*generate_ue_identity_func)(s_stmsi*));
int send_rrc_setup_complete(int socketfd, s_message* message);
int receive_rrc_setup(int socketfd, s_message* received, s_message* message);

#endif