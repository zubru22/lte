#include "logs.h"
/**
 * @brief This function prints out message to stdout as well as to the file specified by first parameter. 
 * 
 * @param log_file_name is path to file in which logs should be saved. File will be created if it does not exist yet.
 * @param log is enum flag which adds tag next to log date, e.g. [SUCCESS] or [FAIL]
 * @param str is message displayed to stdout and written to file, new line character is added automatically
 */
void add_log(const char *log_file_name, int log, const char *str)
{
    FILE *log_file;
    time_t log_time;
    char *local_time;

    // Get current time in seconds
    time(&log_time);
    // Convert it into human readable format
    local_time = asctime(localtime(&log_time));
    // Remove trailing new line character
    strtok(local_time, "\n");
    // Open the file for appending
    log_file = fopen(log_file_name, "a");
    
    // Print log time to terminal and to log file
    printf("%s", local_time);
    fprintf(log_file, "%s", local_time);

    // Print message tag to terminal and to log file
    switch (log) {
        case 0:
            printf(" [SUCCESS] ");
            fprintf(log_file, "[SUCCESS] ");
            break;
        case 1:
            printf(" [WARNING] ");
            fprintf(log_file, "[WARNING] ");
            break;
        case 2:
            printf(" [ERROR] ");
            fprintf(log_file, " [ERROR] ");
            break;
        case 3:
            printf(" [INFO] ");
            fprintf(log_file, " [INFO] ");
            break;
    }

    // Print message description to terminal and to log file
    printf("%s\n", str);
    fprintf(log_file, "%s\n", str);    

    fclose(log_file);
}

void add_logf(const char *log_file_name, int log, const char *str, ...)
{
    FILE *log_file;
    time_t log_time;
    char *local_time;

    // Get current time in seconds
    time(&log_time);
    // Convert it into human readable format
    local_time = asctime(localtime(&log_time));
    // Remove trailing new line character
    strtok(local_time, "\n");
    // Open the file for appending
    log_file = fopen(log_file_name, "a");
    
    // Print log time to terminal and to log file
    printf("%s", local_time);
    fprintf(log_file, "%s", local_time);

    // Print message tag to terminal and to log file
    switch (log) {
        case 0:
            printf(" [SUCCESS] ");
            fprintf(log_file, "[SUCCESS] ");
            break;
        case 1:
            printf(" [WARNING] ");
            fprintf(log_file, "[WARNING] ");
            break;
        case 2:
            printf(" [ERROR] ");
            fprintf(log_file, " [ERROR] ");
            break;
        case 3:
            printf(" [INFO] ");
            fprintf(log_file, " [INFO] ");
            break;
    }

    // Print message description to terminal and to log file
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    printf("\n");
    va_start(args, str);
    vfprintf(log_file, str, args);
    fprintf(log_file, "\n");
    va_end(args);


    fclose(log_file);
}