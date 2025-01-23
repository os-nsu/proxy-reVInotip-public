/*!
    \file master.h
    \brief master process life cycle
    \version 1.0
    \date 16 Jan 2025

    This file contains main loop interface and callback functions
    for work with life cycle of master process.

    IDENTIFICATION
        src/include/master.h
*/
#include <stdint.h>

//=========START_HOOK INTERFACE=========

typedef void (*Hook)(void);

extern Hook executor_start_hook; //< is called immediately before start main loop of master process

//=========REGISTER_WORKER_HOOK INTERFACE=========

/*!
    Defines process termination behavior
*/
typedef enum RestartConstants {
    NEVER = 0, // Never restart worker's proccess after it termination
    ALWAYS = 1 // Always restart worker's proccess after it termination
} RestartConstants;

/*!
    Create new process for plugin
    \param [in] name name of this proccess
    \param [in] lib_name name of library (plugin) without .so
    \param [in] func_name proccess's entrypoint
    \param [in] restart defines process termination behavior
    \param [in] cooldown time in seconds after which the process will be restarted.
        If restart == NEVER this argument will be ignored.
*/
void register_worker(const char *name, const char *lib_name, const char *func_name, RestartConstants restart, uint32_t cooldown);
