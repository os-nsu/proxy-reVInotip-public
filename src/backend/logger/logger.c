/**
 * @file logger.c
 * @author reVInotip
 * @brief Realization of logger.h interface
 * @version 0.1
 * @date 2025-01-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "../../include/logger.h"
#include "../../include/master.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "../../include/utils/extended_string.h"

// Size of buffer with current time
#define TIME_BUFFER_SIZE   100

// Size of buffer with current date
#define DATE_BUFFER_SIZE   50

// Size of buffer with timezone name
#define OFFSET_BUFFER_SIZE 5

#define LOG_FILE_NAME      "proxy.log"

// if system macros is __USE_MISC undefined
#ifndef __USE_MISC
enum {
    DT_REG = 8,
#define DT_REG DT_REG
    DT_DIR = 4,
#define DT_DIR DT_DIR
};
#endif

/**
 * @brief This struct describes logger
 *
 */
typedef struct Logger {
    FILE *stream; // Log file that is open now
    char *log_file_name;
    int file_size_limit;
} Logger;

Logger *logger;
bool is_logger_initted = false;

/**
 * \brief Check is log file complete
 * \return -1 if error occurred. 1 if file size exced capacity of log files
 * 0 if file is not complete.
 */
static int is_log_file_complete() {
    struct stat file_stat;
    if (stat(logger->log_file_name, &file_stat) == -1) {
        if (errno == ENOENT || errno == EFAULT) // file not exists
            return 0;

        fprintf(stderr, "Error while getting information about log file: %s\n", strerror(errno));
        return -1;
    }

    if (file_stat.st_size >= logger->file_size_limit * 1024 && logger->file_size_limit > 0) // in Kb
        return 1;

    return 0;
}

/**
 * @brief It initializes logger's data (path to directory with log files).
 *
 * @param path - path to directory with log files. Could be NULL if file stream wouldn't be used.
 * @param file_size_limit - maximal size in Kb of log file
 * @return 0 if success, -1 and sets errno if error
 */
int init_logger(char *path, int file_size_limit) {
    if (is_logger_initted) { return -1; }
    if (path == NULL) { return 0; }
    if (file_size_limit <= 0) { return -1; }
  
    logger = (Logger *) malloc(sizeof(Logger));
    if (logger == NULL) { return -1; }

    logger->log_file_name = string_concat(path, LOG_FILE_NAME, '/');
    logger->file_size_limit = file_size_limit;

    if (is_log_file_complete()) {
        logger->stream = fopen(logger->log_file_name, "w");
        if (logger->stream == NULL) {
            fprintf(stderr, "Can not open log file: %s\n", strerror(errno));
            goto ERROR;
        }
    } else {
        logger->stream = fopen(logger->log_file_name, "a");
        if (logger->stream == NULL) {
            fprintf(stderr, "Can not open log file: %s\n", strerror(errno));
            goto ERROR;
        }
    }

    is_logger_initted = true;
    return 0;

ERROR:
    free(logger);

    return -1;
}

// Get string representation of LogLevel
char *get_str_loglevel(const enum LogLevel level) {
    switch (level) {
        case LOG_DEBUG:
            return "LOG_DEBUG";
        case LOG_INFO:
            return "LOG_INFO";
        case LOG_WARNING:
            return "LOG_WARNING";
        case LOG_ERROR:
            return "LOG_ERROR";
        case LOG_FATAL:
            return "LOG_FATAL";
        default:
            return NULL;
    }
}


/*!
    This function write message to .log file with specified format (see file
    description)
    \param [in] stream type of output stream
    \param [in] level level of message
    \param [in] filename name of code file where write_log was called
    \param [in] line_number number of line where write_log was called
    \param [in] format pointer to format string (like in printf)
    \param [in] ... variadic list of parameters for format string (like in
    printf)
    \return void
 */
void write_log(enum OutputStream stream, enum LogLevel level, const char *filename, int line_number, const char *format,
               ...) {
    if (is_log_file_complete()) {
        logger->stream = freopen(logger->log_file_name, "w", logger->stream);
        if (logger->stream == NULL) {
            fprintf(stderr, "Can not reopen log file: %s. Change log stream to default (=stderr)\n", strerror(errno));
            is_logger_initted = false;
        }
    }

    va_list args;
    va_start(args, format);

    time_t log_time = time(NULL);
    struct tm *now = localtime(&log_time);

    char time[TIME_BUFFER_SIZE] = {'\0'};
    strftime(time, sizeof(time), "%T", now);

    char date[DATE_BUFFER_SIZE] = {'\0'};
    strftime(date, sizeof(time), "%d/%m/%y", now);

    char utc_offset[OFFSET_BUFFER_SIZE] = {'\0'};
    strftime(utc_offset, sizeof(time), "%Z", now);

    va_end(args);

    FILE *output_stream = NULL;
    if (stream == STDOUT) {
        output_stream = stdout;
    } else if (stream == STDERR) {
        output_stream = stderr;
    } else if (stream == FILESTREAM && is_logger_initted) {
        output_stream = logger->stream;
    } else {
        fprintf(stderr, "Wrong stream parameter! Default value (=stderr) will be used\n");
        output_stream = stderr;
    }

    fprintf(output_stream, "%s %s %s %s %d [%d] | %s: ", date, time, utc_offset, filename, line_number, getpid(),
            get_str_loglevel(level));
    vfprintf(output_stream, format, args);

    fflush(output_stream);

    if (level == LOG_FATAL) shutdown(14);
}

/*!
    Function frees logger's data structures
    @return -1 if logger already finished or 0 if all is OK
     -2 means error
*/
int fini_logger(void) {
    if (!is_logger_initted) { return -1; }

    if (fclose(logger->stream) == EOF) {
        fprintf(stderr, "Can not close current log file: %s\n", strerror(errno));
        return -1;
    }

    free(logger->log_file_name);
    free(logger);
    
    is_logger_initted = false;
    return 0;
}
