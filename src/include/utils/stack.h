/**
 * stack.h
 * Structure describing the stack and methods for it.
 * I use it to store handles of shared libraries.
 */

/**
 * @file stack.h
 * @author reVInotip
 * @brief Structure describing the stack and methods for it.
 * I use it to store handles of shared libraries.
 * @version 0.1
 * @date 2025-01-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <stdlib.h>

#pragma once

typedef struct StackElem
{   
    void *data;
    struct StackElem *next_elem;  
} StackElem;

typedef StackElem *StackPtr;

extern StackPtr create_stack();
extern void push_to_stack(StackPtr *stack, void *element);
extern void* pop_from_stack(StackPtr *stack);
extern void *stack_top(StackPtr stack);
extern void destroy_stack(StackPtr *stack);
extern size_t get_stack_size(StackPtr stack);