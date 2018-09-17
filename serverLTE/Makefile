#declare variables
CC = gcc

CFLAGS=-g -Wall -c
SFLAGS=-g -Wall 

DEPS= . client_handler.h client_object.h packet_handler.h server.h

all: clean create 							## first, clean all the *.o binaries,then compile with new ones.

run:
	@read -p "Enter server port:" port; \
	./server $$port

valgrind: 
	sudo apt install valgrind 				## install valgrind/ check if exists.
	@read -p "Enter server port:" port; \
	valgrind ./server $$port

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(SFLAGS)

create: main.o server.o packet_handler.o hashmap.o MurmurHash3.o client_object.o client_handler.o logs.o $(DEPS)
		
		$(CC) $(SFLAGS) main.o server.o packet_handler.o hashmap.o MurmurHash3.o client_object.o client_handler.o logs.o -I $(DEPS) -lpthread -o server 2> server_output.txt 

main.o: main.c
		$(CC) $(CFLAGS) main.c

server.o: server.c
		$(CC) $(CFLAGS) server.c

packet_handler.o: packet_handler.c
		$(CC) $(CFLAGS) packet_handler.c
		
		##hashmap##

hashmap.o: ../lib/hashmap.c
		$(CC) $(CFLAGS) ../lib/hashmap.c

MurmurHash3.o: ../lib/MurmurHash3.c
		$(CC) $(CFLAGS) ../lib/MurmurHash3.c
	
		##########
client_object.o: client_object.c
		$(CC) $(CFLAGS) client_object.c

client_handler.o: client_handler.c
		$(CC) $(CFLAGS) client_handler.c

logs.o: ../logs/logs.c
	$(CC) $(CFLAGS) ../logs/logs.c

.PHONY : clean
clean: 
	rm server client_handler.o client_object.o MurmurHash3.o hashmap.o \
	packet_handler.o server.o main.o 