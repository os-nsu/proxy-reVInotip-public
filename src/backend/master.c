/**
 * @file master.c
 * @author reVInotip
 * @brief Main loop of program
 * @version 0.1
 * @date 2025-01-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#define PLUGINS_DIR "plugins"

#include "../include/master.h"
#include "../include/logger.h"
#include <stdio.h>
#include <string.h>
#include "../include/plugins_manager.h"
#include "../include/config.h"
#include "../include/utils/extended_string.h"

Hook executor_start_hook = NULL;

int main(int argc, char *argv[]) {
    if (init_logger(NULL, 0) < 0) {
        fprintf(stderr, "Failed to initialize the logger\n");
    }

    if (create_config_table() < 0) {
        fprintf(stderr, "Failed to initialize the config\n");
    }

    // create full path to plugins
    char *source_dir_path = erase_right(argv[0], '/');
    if (source_dir_path != NULL) {
        char *plugins_path = string_concat(source_dir_path, PLUGINS_DIR, '/');
        if (plugins_path == NULL) {
            fprintf(stderr, "Failed to create path to plugins\n");
        } else {
            LOADER(plugins_path);
            if (count_loaded_plugins() > 0) {
                init_all_plugins();
            } else {
                fprintf(stderr, "No extensions have been downloaded\n");
            }

            free(plugins_path);
        }

        free(source_dir_path);
    } else {
        fprintf(stderr, "Failed to get source dir from argv[0]\n");
    }

    close_all_plugins();
    destroy_config_table();
    fini_logger();

    return 0;
}
