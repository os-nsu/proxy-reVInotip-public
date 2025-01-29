/**
 * @file loader.c
 * @author reVInotip
 * @brief Realization of plugins_manager.h interface
 * @version 0.1
 * @date 2025-01-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "../../include/plugins_manager.h"
#include <assert.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/master.h"
#include "../../include/utils/stack.h"
#include "../../include/utils/extended_string.h"
#include "../../include/logger.h"

// if system macros is __USE_MISC undefined
#ifndef __USE_MISC
enum {
    DT_DIR = 4,
#define DT_DIR DT_DIR
    DT_REG = 8,
#define DT_REG DT_REG
};
#endif

typedef struct Plugin {
    void *handle;
    char *name;
} Plugin;

//========UTILS========

/**
 * @brief Retuns plugin index in array. If plugin not in array return -1
 *
 * @param plug_name - searching plugin name
 * @param plugins - array of plugins
 * @param plug_arr_size - size of this array
 * @return int
 */
int plugin_index_in_array(char *plug_name, char **plugins, const size_t plug_arr_size) {
    for (size_t i = 0; i < plug_arr_size; ++i) {
        if (!strcmp(plug_name, plugins[i])) return i;
    }

    return -1;
}

//========INIT PLUGINS MANAGER========

/**
 * @brief Stack with plugins handlers
 */
static StackPtr plug_stack;

/**
 * \brief Init loader (create stack for extensions)
 */
void init_loader() {
    plug_stack = create_stack();
}

//========LOAD PLUGINS========

/**
 * @brief Returns default name for plugin
 *
 * @return const char*
 */
const char *default_plugin_name() {
    return "undefined";
}

/**
 * @brief Recursive function for load plugins
 *
 * @param path_to_source - directory which contains shared libraries (.so files)
 * @param curr_depth  - current scan depth relative to the start directory (0 by default; this parameter need for
 * recursion)
 * @param depth - max scan depth relative to the start directory
 * @param plugins - array of plugins names for load (with .so)
 * @param count_plugins - size of array of plugins names
 */
static void load_plugins(char *path_to_source, int curr_depth, const int depth, char **plugins,
                         const size_t count_plugins) {
    DIR *source = opendir(path_to_source);
    if (source == NULL) {
        LOG(LOG_ERROR, "Error during opening the extensions directory (%s): %s\n", path_to_source,
                strerror(errno));
        return;
    }
    struct dirent *entry;

    // Scan current directory
    while ((entry = readdir(source)) != NULL) {
        // create full path to sample
        char *full_path = string_concat(path_to_source, entry->d_name, '/');
        if (full_path == NULL) {
            free(full_path);
            break;
        }

        int plug_index = plugin_index_in_array(entry->d_name, plugins, count_plugins);

        // if file is regular file and plugins array contains it
        // we load it to RAM
        if (entry->d_type == DT_REG && plug_index >= 0) {
            // load library and push it to stack with libraries
            void *library = dlopen(full_path, RTLD_LAZY);
            if (library == NULL) {
                LOG(LOG_ERROR, "Library couldn't be opened.\n\
                    \tLibrary's path is %s\n\
                    \tdlopen: %s\n\
                    \tcheck plugins folder or rename library\n",
                        path_to_source, dlerror());
            }

            Plugin *plugin = (Plugin *) malloc(sizeof(Plugin));
            plugin->handle = library;
            plugin->name = strndup(plugins[plug_index], strlen(plugins[plug_index]) - 3);

            push_to_stack(&plug_stack, plugin);
            LOG(LOG_INFO, "Plugin %s loaded\n", plugin->name);

            free(full_path);
            break;
        }
        // if file is directory and it is not link to this or previous directory
        else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            load_plugins(full_path, curr_depth + 1, depth, plugins, count_plugins);
        }

        free(full_path);
    }

    closedir(source);
}

/**
 * @brief Find and load all shared libraries
 * @param [in] path_to_source - directory which contains shared libraries (.so files)
 * @param [in] curr_depth - current scan depth relative to the start directory (0 by default; this parameter need for
 * recursion)
 * @param [in] depth - max scan depth relative to the start directory
 * @return nothing
 */
void default_loader(char *path_to_source, int curr_depth, const int depth) {
    if (path_to_source == NULL) {
        LOG(LOG_ERROR, "Can not load extensions because path to them is invalid!\n");
        return;
    }

    if (curr_depth > depth) { return; }

    // It will be replaced in lab2
    size_t count_plugins = 1;
    char *plugin_names[1] = {"greeting"};
    // get_config_parameter("plugins", C_MAIN);

    char **plugins = (char **) calloc(count_plugins, sizeof(char *));
    if (plugins == NULL) { goto FREE_PLUGINS_ARR; }
    // This cycle creates full plugin libs names (and saves to heap)
    for (size_t i = 0; i < count_plugins; ++i) {
        plugins[i] = string_concat(plugin_names[i], "so", '.');
        if (plugins[i] == NULL) {
            count_plugins = i + 1;
            goto FREE_PLUGINS;
        }
    }

    load_plugins(path_to_source, curr_depth, depth, plugins, count_plugins);

FREE_PLUGINS:
    for (size_t i = 0; i < count_plugins; ++i) { free(plugins[i]); }
FREE_PLUGINS_ARR:
    free(plugins);
}

//========INIT PLUGINS========

/**
    @brief Initialize all plugins (call init function)
*/
void init_all_plugins() {
    if (plug_stack == NULL) { return; }

    StackPtr curr_stack = plug_stack;

    for (size_t i = 0; i < get_stack_size(plug_stack); i++) {
        void (*function)() = dlsym(((Plugin *) curr_stack->data)->handle, "init");
        if (function == NULL) {
            LOG(LOG_ERROR, "Library couldn't execute %s.\n\
                    \tLibrary's name is %s. Dlsym message: %s\n\
                    \tcheck plugins folder or rename library\n",
                    "init", ((Plugin *) curr_stack->data)->name, dlerror());
            curr_stack = curr_stack->next_elem;
            continue;
        }
        function();

        curr_stack = curr_stack->next_elem;
    }

    if (executor_start_hook) { executor_start_hook(); }
}

//========CLOSE PLUGINS========

/**
    @brief Close all extensions, unload it from memory and free stack with it
    @return nothing
*/
void close_all_plugins() {
    if (plug_stack == NULL) { return; }

    StackPtr curr_stack = plug_stack;
    for (size_t i = 0; i < get_stack_size(plug_stack); i++) {
        void (*function)() = dlsym(((Plugin *) curr_stack->data)->handle, "fini");
        if (function == NULL) {
            LOG(LOG_ERROR, "Library couldn't execute %s.\n\
                    \tLibrary's name is %s. Dlsym message: %s\n\
                    \tcheck plugins folder or rename library\n",
                    "fini", ((Plugin *) curr_stack->data)->name, dlerror());
        } else {
            function();
        }

        dlclose(((Plugin *) curr_stack->data)->handle);

        free(((Plugin *) curr_stack->data)->name);
        free(curr_stack->data);

        curr_stack = curr_stack->next_elem;
    }

    destroy_stack(&plug_stack);
}

//========OVER========

/**
 * @brief Returns count of loaded plugins. If plugin loading procedure has not been called before
 * or no plugins has been loaded, it returns zero
 * @return size_t
 */
size_t count_loaded_plugins() {
    return get_stack_size(plug_stack);
}
