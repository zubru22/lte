/**
    @file logs.h
    @brief Functions for handling logs both on server and client side 
*/
/**
    @fn void add_log(const char *log_file_name, int log, const char *str)
    @brief This function prints log to stdout as well as to log file
    @param log_file_name Path to log file in which logs will be saved
*/
/**
    @enum logs
    @brief Type of log which occurs, adds [FLAG] to output e.g.
*/
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