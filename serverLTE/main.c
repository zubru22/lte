#ifndef SERVER_INIT_H
#include "server_init.h"
#endif

#ifndef PACKET_HANDLER_H
#include "packet_handler.h"
#endif
#include <stdio.h>

#ifndef CLIENT_H
#include "client.h"
#endif

int main(int argc, char** argv) {
    init_server(atoi(argv[1]));
    receive_packets();
    
    // TODO move to kind of "clean" or "destroy" method
    hashmap_destroy(clients);
}
