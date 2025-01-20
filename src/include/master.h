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

typedef void (*Hook)(void);

extern Hook executor_start_hook; //< is called immediately before start main loop of master process
