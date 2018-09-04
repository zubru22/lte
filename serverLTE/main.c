#include "server_init.h"
#include <stdio.h>

int main(int argc, char** argv) {
  init_server(atoi(argv[1]));
  receive_packets();
}
