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
    fprintf(log_file, "%s", local_time);

    // Print message tag to log file
    switch (log) {
        case 0:
            fprintf(log_file, "[SUCCESS] ");
            break;
        case 1:
            fprintf(log_file, "[WARNING] ");
            break;
        case 2:
            fprintf(log_file, " [ERROR] ");
            break;
        case 3:
            fprintf(log_file, " [INFO] ");
            break;
    }

    // Print message description to log file
    va_list args;
    va_start(args, str);
    va_start(args, str);
    vfprintf(log_file, str, args);
    fprintf(log_file, "\n");
    va_end(args);
}

FILE* log_init(const char* filename, const char* mode) {
    return fopen(filename, mode);
}