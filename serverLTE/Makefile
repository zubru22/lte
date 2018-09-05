all: server
server: main.c server_init.c
	gcc -g -Wall main.c server_init.c -o server 2> server_output.txt
run:
	./server
