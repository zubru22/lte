#ifndef LOGS_H
#include "logs.h"
#endif

void add_logf(FILE* log_file, int log, const char *str, ...)
{
    time_t log_time;
    char *local_time;

    // Get current time in seconds
    time(&log_time);
    // Convert it into human readable format
    local_time = asctime(localtime(&log_time));
    // Remove trailing new line character
    strtok(local_time, "\n");
    
    // Print log time to terminal and to log file
    if(should_print_to_console)
        printf("%s", local_time);
    fprintf(log_file, "%s", local_time);

    // Print message tag to log file
    char* log_type;
    switch (log) {
        case 0:
            log_type = " [SUCCESS] ";
            break;
        case 1:
            log_type = " [WARNING] ";
            break;
        case 2:
            log_type = " [ERROR] ";
            break;
        case 3:
            log_type = " [INFO] ";
            break;
    }
    if(should_print_to_console)
        printf("%s", log_type);
    fprintf(log_file, log_type);

    // Print message description to log file
    va_list args;
    if(should_print_to_console) {    
        va_start(args, str);
        vprintf(str, args);
        printf("\n");
    }
    va_start(args, str);
    vfprintf(log_file, str, args);
    fprintf(log_file, "\n");
    va_end(args);
}

FILE* log_init(const char* filename, const char* mode) {
    return fopen(filename, mode);
    should_print_to_console = false;
}

void set_console_logging(bool value) {
    should_print_to_console = value;
}