/**
 * @file hash_map.h
 * @author reVInotip
 * @brief
 * Structure describing the associative array and methods for it.
 * Implemented based on hash tables. A polynomial hash was used as a hash function.
 * To protect against collisions and add multiple values ​​with the same key, a priority
 * queue of a not fixed size is used (this structure was chosen to support working with the cache).
 *
 * @version 0.1
 * @date 2025-01-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <stdlib.h>

#pragma once

/**
 * @brief Structure describing the priority queue used to prevent collisions and
 *  add multiple values with the same key
 */
typedef struct CollisionsListElement {
    // String representation of a key
    char *key_str;
    void *value;
    struct CollisionsListElement *next;
    struct CollisionsListElement *prev;
} CollisionsListElement;

typedef struct CollisionsList {
    size_t size;
    CollisionsListElement *data;
} CollisionsList;

typedef struct HashMapElem {
    CollisionsList *values;
} HashMapElem;

typedef HashMapElem *HashMapPtr;

extern HashMapPtr create_map();
extern void *get_map_element(HashMapPtr map, const char *key);
/**
 * @brief Push element to hash map
 * @note This function makes copy of elements data, so you can push elements from stack too
 */
extern int push_to_map_copy(HashMapPtr map, const char *key, const void *value, const size_t value_size);
extern int push_to_map(HashMapPtr map, const char *key, void *value);
extern void destroy_map(HashMapPtr *map);
extern void destroy_map_with_data(HashMapPtr *map, void (*data_destroyer)(void *data));
extern size_t get_map_size(HashMapPtr map);
