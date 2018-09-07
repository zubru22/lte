#include "server_init.h"
#include <stdio.h>

#ifndef CLIENT_H
#include "client.h"
#endif

int main(int argc, char** argv) {
  if (argc < 2) {
    remind_about_port();
  }
  init_server(atoi(argv[1]));
  receive_packets();
  // TODO move to kind of "clean" or "destroy" method
  hashmap_destroy(clients);
}
