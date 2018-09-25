/* This functionality is about cellphone menu simulation */
#ifndef DISPLAY_H
#include "display.h"
#endif
#include <string.h>

static char client_log_filename[] = "../logs/client.log";

void display_logs() {
    FILE* log_file;
    if((log_file = fopen(client_log_filename, "r")) == NULL) {
        printf("Dupa najwieksza");
    }
    char line[1024];
    printf("dupa\n");
    
    fseek(log_file, 0, SEEK_SET); // Just to make sure
    while(fscanf(log_file, "%s", line) != EOF) {
        printf("%s", line);
        printf("dupadupa\n");
        memset((char*)line, '\0', sizeof(line));
    }
    fclose(log_file);
}

void display_menu() {
    printf("Menu:\n");
    printf("1) Display logs.\n");
    printf("2) Send SMS.\n");
    printf("3) Received SMS.\n");
}