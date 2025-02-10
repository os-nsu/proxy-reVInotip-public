/**
 * @file hash_map.c
 * @author reVInotip
 * @brief This file contains realization of hah_map.h
 * @version 0.1
 * @date 2025-01-30
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "../../include/utils/hash_map.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/logger.h"

// Max count hash keys in hash table
#define COUNT_ELEMENTS_IN_ARRAY 256

// ======== auxiliary structure methods ===========

static CollisionsList *create_collisions_list() {
    CollisionsList *clist = (CollisionsList *) malloc(sizeof(CollisionsList));
    if (clist == NULL) {
        LOG(LOG_ERROR, "Error wile allocating memory: %s\n", strerror(errno));
        return NULL;
    }

    clist->data = NULL;
    clist->size = 0;
    return clist;
}

static int insert_to_collisions_list(CollisionsList *clist, void *value, const char *key) {
    if (clist == NULL) {
        LOG(LOG_ERROR, "Invalid parameter: clist. It should be not NULL\n");
        return -1;
    }

    CollisionsListElement *new = (CollisionsListElement *) malloc(sizeof(CollisionsListElement));
    if (new == NULL) {
        LOG(LOG_ERROR, "Error wile allocating memory: %s\n", strerror(errno));
        return -1;
    }

    new->next = NULL;

    new->key_str = calloc(strlen(key) + 1, sizeof(char));
    if (new->key_str == NULL) {
        free(new);
        LOG(LOG_ERROR, "Error wile allocating memory: %s\n", strerror(errno));
        return -1;
    }
    strcpy(new->key_str, key);

    new->value = value;

    if (clist->data == NULL) {
        new->prev = NULL;
        clist->data = new;
    } else {
        CollisionsListElement *last_list_elem = clist->data;

        while (last_list_elem->next != NULL) { last_list_elem = last_list_elem->next; }

        new->prev = last_list_elem;
        last_list_elem->next = new;
    }

    clist->size++;

    return 0;
}

static int insert_copy_to_collisions_list(CollisionsList *clist, const void *value, const size_t value_size,
                                          const char *key) {
    if (clist == NULL) {
        LOG(LOG_ERROR, "Invalid parameter: clist. It should be not NULL\n");
        return -1;
    }

    CollisionsListElement *new = (CollisionsListElement *) malloc(sizeof(CollisionsListElement));
    if (new == NULL) {
        LOG(LOG_ERROR, "Error wile allocating memory: %s\n", strerror(errno));
        return -1;
    }

    new->next = NULL;

    new->key_str = calloc(strlen(key) + 1, sizeof(char));
    if (new->key_str == NULL) {
        free(new);
        LOG(LOG_ERROR, "Error wile allocating memory: %s\n", strerror(errno));
        return -1;
    }
    strcpy(new->key_str, key);

    new->value = calloc(value_size, sizeof(char));
    if (new->value == NULL) {
        free(new->key_str);
        free(new);
        LOG(LOG_ERROR, "Error wile allocating memory: %s\n", strerror(errno));
        return -1;
    }
    memcpy(new->value, value, value_size);

    if (clist->data == NULL) {
        new->prev = NULL;
        clist->data = new;
    } else {
        CollisionsListElement *last_list_elem = clist->data;

        while (last_list_elem->next != NULL) { last_list_elem = last_list_elem->next; }

        new->prev = last_list_elem;
        last_list_elem->next = new;
    }

    clist->size++;

    return 0;
}

static CollisionsListElement *get_collisions_list_elem(CollisionsList clist, const char *key) {
    while (clist.data != NULL) {
        if (!strcmp(clist.data->key_str, key)) { return clist.data; }

        clist.data = clist.data->next;
    }

    return NULL;
}

static void delete_from_collisions_list(CollisionsList *clist, CollisionsListElement *del_elem) {
    if (del_elem->prev != NULL) {
        del_elem->prev->next = del_elem->next;
    } else {
        clist->data = del_elem->next;
    }

    if (del_elem->next != NULL) { del_elem->next->prev = del_elem->prev; }

    free(del_elem->value);
    free(del_elem->key_str);
    free(del_elem);
    clist->size--;
}

static void destroy_collisions_list(CollisionsList *clist) {
    CollisionsListElement *curr_elem = clist->data;
    CollisionsListElement *del_elem = clist->data;
    while (clist->data != NULL) {
        curr_elem = curr_elem->next;
        delete_from_collisions_list(clist, del_elem);
        del_elem = curr_elem;
    }

    free(clist);
}

static void destroy_collisions_list_with_data(CollisionsList *clist, void (*data_destroyer)(void *data)) {
    CollisionsListElement *curr_elem = clist->data;
    CollisionsListElement *del_elem = clist->data;
    while (clist->data != NULL) {
        curr_elem = curr_elem->next;
        data_destroyer(del_elem->value);
        delete_from_collisions_list(clist, del_elem);
        del_elem = curr_elem;
    }

    free(clist);
}

static inline size_t get_collisions_list_size(CollisionsList clist) {
    return clist.size;
}

// ======== hash-map methods ===========
/**
 * @brief A remainder function that provides a positive result when the divisor is positive.
 *
 * @param dividend
 * @param divider
 * @return long long
 */
static long long mod(long long dividend, long long divider) {
    long long result = dividend % divider;
    if (result * divider < 0) { result += divider; }

    return result;
}

// Polynomial hash function.
static int hash_function(const char *string) {
    const long long module = 1e9 + 7;
    const long long base = 31;

    long long hash = 0;
    for (int i = 0; string[i] != '\0'; ++i) {
        long long sym_code = (long long) (string[i] - 'a' + 1);
        hash = mod(hash * base + sym_code, module);
    }

    /**
     * The value of a polynomial hash function is mapped
     * to a set to reduce the size of the hash table array.
     */
    return (int) (hash % COUNT_ELEMENTS_IN_ARRAY);
}

HashMapPtr create_map() {
    HashMapPtr map = (HashMapPtr) calloc(COUNT_ELEMENTS_IN_ARRAY, sizeof(HashMapElem));
    if (map == NULL) {
        LOG(LOG_ERROR, "Can not create hash map: %s\n", strerror(errno));
        return NULL;
    }

    for (size_t i = 0; i < COUNT_ELEMENTS_IN_ARRAY; i++) {
        map[i].values = create_collisions_list();
        if (map[i].values == NULL) {
            LOG(LOG_ERROR, "Can not create collisions list for hash map: %s\n", strerror(errno));

            for (int j = 0; j < i; ++j) { free(map[j].values); }
            free(map);
            return NULL;
        }
    }

    return map;
}

void *get_map_element(HashMapPtr map, const char *key) {
    int key_index = hash_function(key);
    CollisionsListElement *elem = get_collisions_list_elem(*map[key_index].values, key);
    if (elem == NULL) { return NULL; }

    return elem->value;
}

/**
 * @brief Push element to hash map
 * @note This function makes copy of elements data, so you can push elements from stack too
 */
int push_to_map_copy(HashMapPtr map, const char *key, const void *value, const size_t value_size) {
    int key_index = hash_function(key);
    return insert_copy_to_collisions_list(map[key_index].values, value, value_size, key);
}

int push_to_map(HashMapPtr map, const char *key, void *value) {
    int key_index = hash_function(key);
    return insert_to_collisions_list(map[key_index].values, value, key);
}

void destroy_map(HashMapPtr *map) {
    for (size_t i = 0; i < get_map_size(*map); i++) { destroy_collisions_list((*map)[i].values); }

    free(*map);
}

void destroy_map_with_data(HashMapPtr *map, void (*data_destroyer)(void *data)) {
    for (size_t i = 0; i < get_map_size(*map); i++) {
        destroy_collisions_list_with_data((*map)[i].values, data_destroyer);
    }

    free(*map);
}

size_t get_map_size(HashMapPtr map) {
    return COUNT_ELEMENTS_IN_ARRAY;
}
