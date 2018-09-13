#ifndef SERVER_H
#include "server.h"
#endif
#include <stdio.h>
#include <signal.h>

#ifndef CLIENT_OBJECT_H
#include "client_object.h"
#endif

int main(int argc, char** argv) {
  signal(SIGINT, clean);
  if (argc < 2) {
    remind_about_port();
  }
  init_server(atoi(argv[1]));
  receive_packets();
  clean();
}
