#include <stdio.h>
#include <time.h>
#include <string.h>

enum logs {LOG_SUCCESS, LOG_WARNING, LOG_ERROR} type_of_log;
void add_log(const char *log_file_name, int log, const char *str);