#declare variables
CC = gcc

CFLAGS=-g -Wall -c
SFLAGS=-g -Wall 

DEPS= handle_messages.h init_connection.h random_access.h rrc.h shutdown.h user_equipment.h ../logs/logs.h

all: client

run:
	@read -p "Enter port number:" port; \
	./client $$port

valgrind: 
	@read -p "Enter port number:" port; \
	valgrind ./client $$port

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(SFLAGS)

client: main.o handle_messages.o init_connection.o random_access.o rrc.o shutdown.o user_equipment.o logs.o $(DEPS)
		$(CC) $(SFLAGS) main.o handle_messages.o init_connection.o random_access.o rrc.o shutdown.o user_equipment.o logs.o -o client -lpthread

main.o: main.c
		$(CC) $(CFLAGS) main.c

handle_messages.o: handle_messages.c
		$(CC) $(CFLAGS) handle_messages.c

init_connection.o: init_connection.c
		$(CC) $(CFLAGS) init_connection.c
		
random_access.o: random_access.c
		$(CC) $(CFLAGS) random_access.c

rrc.o: rrc.c
		$(CC) $(CFLAGS) rrc.c
	
		##       ##
shutdown.o: shutdown.c
		$(CC) $(CFLAGS) shutdown.c

user_equipment.o: user_equipment.c
		$(CC) $(CFLAGS) user_equipment.c

logs.o: ../logs/logs.c
	$(CC) $(CFLAGS) ../logs/logs.c
