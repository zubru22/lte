/* This functionality is about cellphone menu simulation */
#ifndef DISPLAY_H
#include "display.h"
#endif
#include <string.h>

extern FILE* log_file;

void display_logs() {
    char line[1024];
    
    fseek(log_file, 0, SEEK_SET);
    while(fscanf(log_file, "%s", line) != EOF) {
        printf("%s", line);
        memset((char*)line, '\0', sizeof(line));
    }
}

void display_menu() {
    printf("Menu:\n");
    printf("1) Display logs.\n");
    printf("2) Send SMS.\n");
    printf("3) Received SMS.\n");
}

void display_menu_options() {
    display_menu();

    int option = 0;
    printf("Choose option: ");
    if(1 == scanf("%d", &option)) {
            menu_options = (e_menu_options)option;
            if(menu_options == DISPLAY_LOGS)
                printf("Right\n");
            switch(menu_options) {
            case DISPLAY_LOGS:
                display_logs();
                break;
            case DISPLAY_SEND_SMS:
                // Display send sms here
                break;
            case DISPLAY_RECV_SMS:
                // Display recv sms here
                break;
            default:
                display_menu();
        }
    }
    else {
        printf("Number of option has to be a positive integer bounded between <1;3>!\n");
    }
}