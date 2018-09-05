all: server
server: main.c server_init.c packet_handler.c
	gcc -g -Wall main.c server_init.c packet_handler.c -o server 2> server_output.txt
run:
	./server
