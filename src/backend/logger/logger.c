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

/**
 * @brief  It initializes logger's data (path to directory with log files).
 * 
 * @param path - path to directory with log files. Could be NULL if file stream wouldn't be used.
 * @param file_size_limit - maximal size in Kb of log file 
 * @param files_limit - maximal count of log files in directory (-1 means
    infinity). This parameter wouldn't be checked if path is NULL.
 * @return 0 if success, -1 and sets errno if error
 */
int init_logger(char* path, long file_size_limit, int files_limit) {
    return 0;
}