/**
 * @file config.c
 * @author reVInotip
 * @brief Realization of config.h interface
 * @version 0.1
 * @date 2025-01-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "../../include/config.h"
#include <stdbool.h>

bool is_config_created = false;


/**
 * @brief Create a config table object
 * 
 * @return -1 if table already exists or 0 if all is OK
 */
int create_config_table(void) {
    if (is_config_created) {
        return -1;
    }

    is_config_created = true;
    return 0;
}

/*!
    Destroy config table and frees all resources associated with it. It should
   be called once.
    @return -1 if table already destroyed or 0 if all is OK
*/
int destroy_config_table(void) {
    if (is_config_created) {
        is_config_created = false;
        return 0;
    }

    return -1;
}
