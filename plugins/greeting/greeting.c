#include "greeting.h"
#include "../../src/include/master.h"
#include "stdio.h"

static Hook prev_executor_start_hook = NULL;

void then_start() {
    if (prev_executor_start_hook) { prev_executor_start_hook(); }
    printf("Hello, world!");
}

void init(void) {
    prev_executor_start_hook = executor_start_hook;
    executor_start_hook = then_start;

    printf("greeting initialized");
}

void fini(void) {
    executor_start_hook = prev_executor_start_hook;
}

const char *name(void) {
    return "greeting";
}
