#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef STDIO_H
#include <stdio.h>
#endif

typedef enum _e_menu_options {
    DISPLAY_LOGS,
    DISPLAY_SEND_SMS,
    DISPLAY_RECV_SMS,
    DISPLAY_MENU
} e_menu_options;

e_menu_options menu_options;

void display_menu();
void display_logs(FILE*);
void display_recv_messages();

#endif