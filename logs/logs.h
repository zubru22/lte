/**
    @file logs.h
    @brief Functions for handling logs both on server and client side 
*/
/**
    @fn void add_logf(const char *log_file_name, int log, const char *str, ...)
    @brief This function prints log to stdout as well as to log file
    @param log_file_name Path to log file in which logs will be saved
    @param log Enum value for describing type of event
    @param str Message to be displayed and saved to log file (can be formatted as printf())
*/
/**
    @enum logs
    @brief Flag for type event which occured, e.g. [SUCCESS]
*/
/**
    @var static char client_log_filename
    @brief Relative path to file where logs should be saved
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

enum logs {
    /** Flag for successful events */
    LOG_SUCCESS,
    /** Flag for warnings events */ 
    LOG_WARNING, 
    /** Flag for error events */
    LOG_ERROR, 
    /** Flag for neutral events */
    LOG_INFO
} type_of_log;
static char client_log_filename[] = "../logs/client.log";
static char server_log_filename[] = "../logs/server.log";
void add_logf(const char *log_file_name, int log, const char *str, ...);
#endif