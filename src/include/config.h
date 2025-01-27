/*!
    \file config.h
    \brief Config static library interface
    \version 1.0
    \date 15 Jan 2025

    This file contains config's function signatures and type definitions.

    Config is the singleton. Therefore double create_config_table must return
   -1.

    Configuration file it is set of key-value pair in format: "key = value" (the
   number of spaces and their position of non-significant ones can be any)

    key must consists of this symbols: A-Z, a-z, 0-9, _, - and it length must be
   <= 127

    value must be one of this types:
        INTEGER - signed integer with size 64 bits
        REAL - signed floating point number with double precision
        STRING - c-style const string
    and array of it in format: [value, value, ..., value]. Array can contains
   only values with same type.

    Also config string can constains one-line comment starting with "#".

    Default configuration file name: proxy.conf, it is stored in project's root
   directory.

    TODO error handling and include command for config file
*/

#include <stdbool.h>
#include <stdint.h>

#pragma once

/*!
    Create config table. It should be called once.
    \return -1 if table already exists or 0 if all is OK
*/
int create_config_table(void);