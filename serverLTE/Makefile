#declare variables
CC = gcc

CFLAGS=-g -Wall -c
SFLAGS=-g -Wall 

DEPS= client_handler.h client.h packet_handler.h server.h

all: server


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(SFLAGS)

server: main.o server.o packet_handler.o hashmap.o MurmurHash3.o client.o client_handler.o $(DEPS)
		$(CC) $(SFLAGS) main.o server.o packet_handler.o hashmap.o MurmurHash3.o client.o client_handler.o -lpthread -o server 2> server_output.txt -I $(DEPS)

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
	
		##       ##
client.o: client.c
		$(CC) $(CFLAGS) client.c

client_handler.o: client_handler.c
		$(CC) $(CFLAGS) client_handler.c


run:	
	./server