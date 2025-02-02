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
#include <stdbool.h>

bool is_logger_initted = false;

/*!
    It initializes logger's data (path to directory with log files).
    \param [in] path path to directory with log files. Could be NULL if file stream wouldn't be used.
    \param [in] file_size_limit maximal size in Kb of log file 
    \return 0 if success, -1 and sets errno if error
*/
int init_logger(char* path, int file_size_limit) {
    if (is_logger_initted) {
        return -1;
    }

    is_logger_initted = true;
    return 0;
}

/*!
    Function frees logger's data structures
    @return -1 if logger already finished or 0 if all is OK
*/
int fini_logger(void) {
    if (is_logger_initted) {
        is_logger_initted = false;
        return 0;
    }

    return -1;
}