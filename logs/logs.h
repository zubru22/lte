#ifndef LOGS_H
#define LOGS_H

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#ifndef TIME_H
#define TIME_H
#include <time.h>
#endif

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

#include <stdarg.h>

enum logs {LOG_SUCCESS, LOG_WARNING, LOG_ERROR, LOG_INFO} type_of_log;
static char client_log_filename[] = "../logs/client.log";
static char server_log_filename[] = "../logs/server.log";
void add_log(const char *log_file_name, int log, const char *str);
void add_logf(const char *log_file_name, int log, const char *str, ...);
#endif