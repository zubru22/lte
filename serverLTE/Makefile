all: server
server: main.c server.c packet_handler.c ../lib/hashmap.c ../lib/MurmurHash3.c client.c client_handler.c ../logs/logs.c
	gcc -g -Wall main.c server.c packet_handler.c ../lib/hashmap.c ../lib/MurmurHash3.c client.c client_handler.c ../logs/logs.c -lpthread -o server 2> server_output.txt
run:
	./server
