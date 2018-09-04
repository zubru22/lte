
const int MAX_EVENTS = 100;
const int MAX_CLIENTS = 10;

server_t server;
client_t **clients;

typedef struct {
  int socket;
} client_t;

typedef struct {
  int socket;
  struct sockaddr_in server_address;
} server_t;

void server_t__init(server_t* self, socket, struct sockaddr server_address);
void init_server_address(struct sockaddr_in* server_address, int port);
void init_server(int port);
