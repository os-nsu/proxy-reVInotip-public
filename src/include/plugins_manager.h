/**
 * @file plugins_manager.h
 * @author reVInotip
 * @brief This file contains basic operations on plugins such as
 * load to memory, removal from it and initialization.
 * @version 0.1
 * @date 2025-01-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "utils/stack.h"

#pragma once

/**
 * @brief Find, load plugins and store it to stack. This macros use base function default_loader()
 *   with scan depth = 1
 *
 * @param [in] path_to_source - directory which contains plugins (.so files)
 * @return nothing
 */
#define LOADER(path_to_source)                                                                                         \
    do { default_loader(path_to_source, 0, 1); } while (0)

/**
 * @brief Base function for loading plugins.
 *
 * @param [in] path_to_source - directory which contains plugins (.so files)
 * @param [in] curr_depth - starting scan depth (cout folders before .so relative to the path_to_source)
 * @param [in] depth - max scan depth
 */
extern void default_loader(char *path_to_source, int curr_depth, const int depth);
/**
 * @brief Close plugins and frees up all the resources associated with them
 */
extern void close_all_plugins();
/**
 * @brief Execute init function in all loaded plugins
 */
extern void init_all_plugins();
/**
 * @brief Init loader (create stack for plugins handlers)
 */
extern void init_loader();
/**
 * @brief Get count of loaded plugins
 * 
 * @return Count of loaded plugins. If plugin loading procedure has not been called before
 * or no plugins has been loaded, it returns zero
 */
extern size_t count_loaded_plugins();