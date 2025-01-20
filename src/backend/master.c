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

#define PLUGINS_DIR "install/plugins"

#include "../include/master.h"
#include <stdio.h>
#include <string.h>
#include "../include/plugins_manager.h"

Hook executor_start_hook = NULL;

int main(int argc, char *argv[]) {
    LOADER(PLUGINS_DIR);
    if (count_loaded_plugins() > 0) {
        init_all_plugins();
    } else {
        fprintf(stderr, "[WARN] No extensions have been downloaded\n");
    }

    close_all_plugins();

    return 0;
}
