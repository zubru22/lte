clientmake: handle_messages.c ../logs/logs.c user_equipment.c rrc.c init_connection.c random_access.c main.c
	gcc -o client handle_messages.c ../logs/logs.c user_equipment.c rrc.c init_connection.c random_access.c main.c -I.
	valgrind ./client ${port}
