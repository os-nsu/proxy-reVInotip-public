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

#include "../include/master.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/config.h"
#include "../include/utils/extended_string.h"
#include "../include/logger.h"
#include "../include/plugins_manager.h"
#include <getopt.h>
#include <unistd.h>

#define PLUGINS_DIR "plugins"

Hook executor_start_hook = NULL;

int main(int argc, char *argv[]) {
    char *log_file_path = getenv("LOG_FILE_PATH");

    // parse options
    struct option long_opt[] = {{"log_dir", 1, NULL, 'l'}};
    int option, opt_index;
    while ((option = getopt_long(argc, argv, "l:", long_opt, &opt_index)) != -1) {
        switch (option) {
            case 'l':
                log_file_path = optarg;
                break;
            case '?':
                break;
            default:
                fprintf(stderr, "Unknow option: %c\n", option);
                break;
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc) { printf("%s ", argv[optind++]); }
        printf("\n");
    }

    if (init_logger(log_file_path, 1) < 0) { LOG(LOG_ERROR, "Failed to initialize the logger\n"); }

    if (create_config_table() < 0) { LOG(LOG_FATAL, "Failed to initialize the config\n"); }

    // create full path to plugins
    char *source_dir_path = erase_right(argv[0], '/');
    if (source_dir_path != NULL) {
        char *plugins_path = string_concat(source_dir_path, PLUGINS_DIR, '/');
        if (plugins_path == NULL) {
            fprintf(stderr, "Failed to create path to plugins\n");
        } else {
            // load plugins
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

    shutdown(0);

    return 0;
}

/**
 * @brief Terminates program with some exit code
 * 
 * @param code - exit code
 */
void shutdown(int code) {
    close_all_plugins();
    destroy_config_table();
    fini_logger();

    exit(code);
}
