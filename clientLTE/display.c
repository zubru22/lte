/* This functionality is about cellphone menu simulation */
#ifndef DISPLAY_H
#include "display.h"
#endif
#include <string.h>

static char client_log_filename[] = "../logs/client.log";

void display_logs(FILE* outer_file) {
    fclose(outer_file);
    FILE* log_file;

    if((log_file = fopen(client_log_filename, "r")) == NULL) {
        printf("Error reading log file!");
    }
    char line[1024];
    
    fseek(log_file, 0, SEEK_SET); // Just to make sure

    while(fscanf(log_file, "%[^\n]\n", line) != EOF) {
        printf("%s\n", line);
        memset((char*)line, '\0', sizeof(line));
    }

    fclose(log_file);
    outer_file = fopen(client_log_filename, "a");
}

void display_menu() {
    printf("\t\t\tMenu:\n");
    printf("\t\t\t1) Display logs.\n");
    printf("\t\t\t2) Send SMS.\n");
    printf("\t\t\t3) Received SMS.\n");
}

void display_recv_messages() {
    FILE* log_file;

    if((log_file = fopen("Received_messages", "r")) == NULL) {
        printf("Error reading log file!");
    }
    char line[1024];
    
    fseek(log_file, 0, SEEK_SET); // Just to make sure

    while(fscanf(log_file, "%[^\n]\n", line) != EOF) {
        printf("%s\n", line);
        memset((char*)line, '\0', sizeof(line));
    }

    fclose(log_file);
}