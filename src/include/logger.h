/*!
    \file logger.h
    \brief Logger dynamic library interface
    \version 1.0
    \date 15 Jan 2025

    This file contains logger's function signatures.

    Logger is the singleton. Therefore double init_logger must return -1.
    
    Logger must be able to write messages in some output streams that 
    enumerates in OutputStream enumeration:

    1) STDOUT
    2) STDERR - standard stream for logging. Logger must use this stream 
    if chosen stream not available.
    3) FILE - logger is able to use this stream if parameter path in 
    init_logger call is not NULL. There are log files in chosen directory.
    Their names obey next rule: "proxyN.log", where N is order number of file.
    If all available log files are filled, then logger starts overwrite them
    from first. 

    IDENTIFICATION
        src/include/logger.h
*/

#pragma once

/*!
    It initializes logger's data (path to directory with log files).
    \param [in] path path to directory with log files. Could be NULL if file stream wouldn't be used.
    \param [in] file_size_limit maximal size in Kb of log file 
    \param [in] files_limit maximal count of log files in directory (-1 means
    infinity). This parameter wouldn't be checked if path is NULL.
    \return 0 if success, -1 and sets errno if error
*/
int init_logger(char* path, long file_size_limit, int files_limit);


