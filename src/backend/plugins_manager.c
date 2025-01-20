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

#include "../include/plugins_manager.h"
#include <assert.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/master.h"
#include "../include/utils/stack.h"

// if system macros is __USE_MUSC undefined
#ifndef __USE_MISC
enum {
    DT_DIR = 4,
#define DT_DIR DT_DIR
    DT_REG = 8,
#define DT_REG DT_REG
};
#endif

//========UTILS========

/**
 * @brief Create new string by concatenating two strings divided by separator (-1 means without separator)
 *
 * @return str1 + separator + str2 and NULL if some error occurred
 *
 * @note This function allocate memory in heap and you need to free it
 */
char *string_concat(char *str1, char *str2, char separator) {
    size_t len1 = strlen(str1);
    size_t len = len1 + strlen(str2) + 2; // len of first string + len of second string +
                                          // + separator + end of string ('\0')
    char *new_str = (char *) malloc(len);
    if (new_str == NULL) {
        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        return NULL;
    }

    memset(new_str, '\0', len);
    new_str = strcpy(new_str, str1);

    if (separator >= 0) { new_str[len1] = separator; }

    new_str = strcat(new_str, str2);

    return new_str;
}

bool is_plugin_in_array(char *plug_name, char **plugins, const size_t plug_arr_size) {
    for (size_t i = 0; i < plug_arr_size; ++i) {
        if (!strcmp(plug_name, plugins[i])) return true;
    }

    return false;
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
 * @brief Find and load all shared libraries
 * @param [in] path_to_source - directory which contains shared libraries (.so files)
 * @param [in] curr_depth - current scan depth relative to the start directory (0 by default; this parameter need for
 * recursion)
 * @param [in] depth - max scan depth relative to the start directory
 * @return nothing
 */
void default_loader(char *path_to_source, int curr_depth, const int depth) {
    if (path_to_source == NULL) {
        fprintf(stderr, "[ERROR] Can not load extensions because path to them is invalid!\n");
        return;
    }

    if (curr_depth > depth) { return; }

    /*if (!is_var_exists_in_config("plugins", C_MAIN)) {
        elog(WARN, "Variable plugins does not exists in config");
        return;
    }*/

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

    DIR *source = opendir(path_to_source);
    if (source == NULL) {
        fprintf(stderr, "[ERROR] Error during opening the extensions directory (%s): %s\n", path_to_source,
                strerror(errno));
        goto FREE_PLUGINS;
    }
    struct dirent *entry;

    // Scan current directory
    while ((entry = readdir(source)) != NULL) {
        // create full path to sample
        char *full_path = string_concat(path_to_source, entry->d_name, '/');
        if (full_path == NULL) { break; }

        // if file is regular file and plugins array contains it
        // we load it to RAM
        if (entry->d_type == DT_REG && is_plugin_in_array(entry->d_name, plugins, count_plugins)) {
            // load library and push it to stack with libraries
            void *library = dlopen(full_path, RTLD_LAZY);
            if (library == NULL) { fprintf(stderr, "[WARN] %s\n", dlerror()); }
            push_to_stack(&plug_stack, library);
            break;
        }
        // if file is directory and it is not link to this or previous directory
        else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            default_loader(full_path, curr_depth + 1, depth);
        }

        free(full_path);
    }

    closedir(source);

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
        void (*function)() = dlsym(curr_stack->data, "init");
        if (function == NULL) {
            fprintf(stderr, "[WARN] %s\n", dlerror());
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
        void (*function)() = dlsym(curr_stack->data, "fini");
        if (function == NULL) { fprintf(stderr, "[WARN] %s\n", dlerror()); }
        function();

        if (dlclose(curr_stack->data)) { fprintf(stderr, "[ERROR] Can`t close extension\n"); }

        curr_stack = curr_stack->next_elem;
    }

    destroy_stack(&plug_stack);
}

//========OVER========


/**
 * @brief Get count of loaded plugins
 *
 * @return Count of loaded plugins. If plugin loading procedure has not been called before
 * or no plugins has been loaded, it returns zero
 */
size_t count_loaded_plugins() {
    return get_stack_size(plug_stack);
}