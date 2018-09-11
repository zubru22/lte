#include "server.h"
#include <stdio.h>
#include <signal.h>

#ifndef CLIENT_H
#include "client.h"
#endif

hashmap* clients;

int main(int argc, char** argv) {
  signal(SIGINT, clean);
  if (argc < 2) {
    remind_about_port();
  }
  init_server(atoi(argv[1]));
  receive_packets();
  clean();
}
