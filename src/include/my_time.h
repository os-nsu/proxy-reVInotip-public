/*!
    \file my_time.h
    \brief Time static library interface
    \version 1.0
    \date 16 Jan 2025

    This file contains time library function's signatures.
*/

#include <time.h>

/*!
    A wrapper over a system call time
    \return time as number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC) 
*/
time_t get_time(void);