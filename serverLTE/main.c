#include "server_init.h"
#include <stdio.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    remind_about_port();
  }
  init_server(atoi(argv[1]));
  receive_packets();
}
