/**
 * @file extended_string.c
 * @author reVInotip
 * @brief This file contains realization of extended_string.h
 * @version 0.1
 * @date 2025-01-27
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/utils/extended_string.h"
#include "../../include/logger.h"

/**
 * @brief Create new string by concatenating two strings divided by separator (-1 means without separator)
 *
 * @return str1 + separator + str2 and NULL if some error occurred
 *
 * @warning This function allocate memory in heap and you need to free it
 */
char *string_concat(char *str1, char *str2, char separator) {
    size_t len1 = strlen(str1);
    size_t len = len1 + strlen(str2) + 2; // len of first string + len of second string +
                                          // + separator + end of string ('\0')
    char *new_str = (char *) malloc(len);
    if (new_str == NULL) {
        LOG(LOG_ERROR, "Malloc error %s\n", strerror(errno));
        return NULL;
    }

    memset(new_str, '\0', len);
    new_str = strcpy(new_str, str1);

    if (separator >= 0) { new_str[len1] = separator; }

    new_str = strcat(new_str, str2);

    return new_str;
}

/**
 * @brief Erase all symbols after last erase_after symbol in string (erase_after not include)
 * 
 * @param str 
 * @param erase_after
 * @return char*
 * @warning This function allocate memory in heap and you need to free it
 */
char *erase_right(char *str, char erase_after) {
    int pos = strlen(str);
    while (str[pos] != erase_after) {
        --pos;
        if (pos < 0) { return NULL; }
    }

    char *new_str = (char*)calloc(pos, sizeof(char));
    if (new_str == NULL) {
        LOG(LOG_ERROR, "Calloc error %s\n", strerror(errno));
        return NULL;
    }

    strncpy(new_str, str, pos);

    return new_str;
}

/**
 * @brief Create new string by concatenating some strings divided by separator (-1 means without separator)
 * If count of this strings <= 0 function returns or error occurred NULL.
 * If count == 1 function store the string passed to it in heap (without separator if it was passed) and returns it.
 * If count > 1 function concatenates all strings and insert a separator between them. New string will be stored in heap
 * and will be returned.
 *
 * @return char *
 *
 * @warning This function allocate memory in heap and you need to free it
 */
char *multi_string_concat(char separator, int count, ...) {
    if (count <= 0) { return NULL; }
    va_list args;

    // first step: calculate summary length for new string
    va_start(args, count);

    // count of separators (count - 1) + end of string ('\0') (1) (= count)
    size_t summary_len = count;
    for (int i = 0; i < count; ++i) { summary_len += strlen(va_arg(args, char *)); }

    va_end(args);

    // second step: allocate memory and fill it with zeros
    char *new_str = (char *) calloc(summary_len, sizeof(char));
    if (new_str == NULL) {
        LOG(LOG_ERROR, "Malloc error %s\n", strerror(errno));
        return NULL;
    }

    // third step: make new string by concatenating others
    va_start(args, count);

    if (separator >= 0) {
        for (int i = 0; i < count - 1; ++i) {
            char *curr_str = va_arg(args, char *);
            new_str = strcat(new_str, curr_str);
            new_str[strlen(new_str)] = separator;
        }

        new_str = strcat(new_str, va_arg(args, char *));
    } else {
        for (int i = 0; i < count; ++i) { new_str = strcat(new_str, va_arg(args, char *)); }
    }

    va_end(args);

    return new_str;
}

/**
 * @brief Create new string by concatenating two strings divided by separator (-1 means without separator)
 * @param [in] empty_size - the memory size that will be allocated in excess of the required amount
 * @return str1 + separator + str2 and NULL if some error occurred
 *
 * @note This function allocate memory in heap and you need to free it
 */
char *oversize_string_concat(char *str1, char *str2, char separator, size_t empty_size) {
    size_t len1 = strlen(str1);
    size_t len = len1 + strlen(str2) + 2; // len of first string + len of second string +
                                          // + separator + end of string ('\0')
    char *new_str = (char *) malloc(len + empty_size);
    if (new_str == NULL) {
        LOG(LOG_ERROR, "Malloc error %s\n", strerror(errno));
        return NULL;
    }

    memset(new_str, '\0', len);
    new_str = strcpy(new_str, str1);

    if (separator >= 0) { new_str[len1] = separator; }

    new_str = strcat(new_str, str2);

    return new_str;
}
