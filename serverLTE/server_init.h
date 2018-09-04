
const int MAX_EVENTS = 100;
const int MAX_CLIENTS = 10;

server_t server;

typedef struct {
  int socket;
} client_t;

typedef struct {
  int socket;
  struct sockaddr_in server_address;
  int epoll_file_descriptor;
  struct epoll_event event;
  struct epoll_event events[MAX_EVENTS];
  client_t **clients;
} server_t;

void server_t__init(server_t* self, int socket, struct sockaddr server_address);
int servet_t__socket(server_t* self);
void init_server_address(struct sockaddr_in* server_address, int port);
void init_server(int port);
