/**
 * @file extended_string.h
 * @author reVInotip
 * @brief This file contains extended operations on c-string
 * @version 0.1
 * @date 2025-01-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <stddef.h>

#pragma once


/**
 * @brief Create new string by concatenating two strings divided by separator (-1 means without separator)
 *
 * @return str1 + separator + str2 and NULL if some error occurred
 *
 * @warning This function allocate memory in heap and you need to free it
 */
extern char *string_concat(char *str1, char *str2, char separator);

/**
 * @brief Erase all symbols after last erase_after symbol in string
 * 
 * @param str 
 * @param erase_after 
 * @return char* 
 * @warning This function allocate memory in heap and you need to free it
 */
char *erase_right(char *str, char erase_after);

/**
 * @brief Create new string by concatenating two strings divided by separator (-1 means without separator)
 * @param [in] empty_size - the memory size that will be allocated in excess of the required amount
 * @return str1 + separator + str2 and NULL if some error occurred
 *
 * @warning This function allocate memory in heap and you need to free it
 * @note Need to test
 */
extern char *oversize_string_concat(char *str1, char *str2, char separator, size_t empty_size);

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
 * @note Need to test
 */
extern char *multi_string_concat(char separator, int count, ...);