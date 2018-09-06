#ifndef SERVER_INIT_H
#include "server_init.h"
#endif

#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif
#include <stdio.h>


int main(int argc, char** argv) {
    init_server(atoi(argv[1]));
    receive_packets();
}
