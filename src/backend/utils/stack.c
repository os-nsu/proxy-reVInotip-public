/**
 * @file stack.c
 * @author reVInotip
 * @brief This file contains realization of stack.h
 * @version 0.1
 * @date 2025-01-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "../../include/utils/stack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

inline StackPtr create_stack() {
    return NULL;
}

void *stack_top(StackPtr stack) {
    assert(stack != NULL);

    return stack->data;
}

void push_to_stack(StackPtr *stack, void *element) {
    StackElem *new_element = (StackElem *) malloc(sizeof(StackElem));
    assert(new_element != NULL);

    new_element->data = element;
    new_element->next_elem = *stack;
    *stack = new_element;
}

void *pop_from_stack(StackPtr *stack) {
    assert(stack != NULL);

    void *elem_data = (*stack)->data;
    StackElem *elem_for_erase = *stack;
    *stack = (*stack)->next_elem;
    free(elem_for_erase);
    return elem_data;
}

void destroy_stack(StackPtr *stack) {
    assert(stack != NULL);

    StackPtr curr_stack = NULL;
    while (*stack != NULL) {
        curr_stack = *stack;
        *stack = (*stack)->next_elem;
        free(curr_stack);
    }
}

size_t get_stack_size(StackPtr stack) {
    if (stack == NULL) { return 0; }
    size_t size = 0;

    StackPtr curr_stack = stack;
    while (curr_stack != NULL) {
        ++size;
        curr_stack = curr_stack->next_elem;
    }

    return size;
}
