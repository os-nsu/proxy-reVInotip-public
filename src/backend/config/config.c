/**
 * @file config.c
 * @author reVInotip
 * @brief Realization of config.h interface
 * @version 0.1
 * @date 2025-01-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/**
 * TODO
 *  Split analize_config_string function with read_config_string function
 */
#include "../../include/config.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/logger.h"
#include "../../include/utils/hash_map.h"
#include "../../include/utils/stack.h"

#define MAX_CONFIG_KEY_SIZE     128
#define START_CONFIG_VALUE_SIZE 200

#define UNDEF_VAR_NAME          "_UNDEFINED_VARIABLE_"
#define UNDEF_VAR_DESCR         "This variable is base undefined variable"

#define STANDARD_DESCRIPTION    "Hello there!"

static HashMapPtr map;
static StackPtr alloc_mem_stack;

bool is_config_created = false;

typedef struct ConfigString {
    char key[MAX_CONFIG_KEY_SIZE];
    ConfigData data; // variable content. It can be single or array. You should
                     // look at count filed to determine it.
    ConfigVarType type; // variable type. If it is UNDEFINED, then error
                        // occurred and another fields are meanless
    int count; // count items in array. If it is one, then variable is single.
} ConfigString;

//=========UTILS (constructors and destructors for config variables and config variables data)=========

static bool is_data_exists(const ConfigVarType type, const ConfigData validated_data) {
    switch (type) {
        case UNDEFINED:
            LOG(LOG_WARNING, "Try to check data for UNDEFINED variable\n");
            return false;
        case INTEGER:
            return validated_data.integer != NULL;
        case REAL:
            return validated_data.real != NULL;
        case STRING:
            return validated_data.string != NULL;
    }
}

/**
 * @brief Create a variable data object
 *
 * @param type
 * @param data
 * @param data_type - 1 - raw string array (char **), 2 - type of data parameter matches with type paramater. Another
 * value means ignore data parameter
 * @param count
 * @return ConfigData
 */
static ConfigData create_var_data(const ConfigVarType type, void *data, const int data_type, const size_t count) {
    ConfigData unnamed_data;
    char **str_data = (char **) data;

    switch (type) {
        case INTEGER:
            unnamed_data.integer = (int64_t *) calloc(count, sizeof(int64_t));
            if (unnamed_data.integer == NULL) {
                LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
                return unnamed_data;
            }

            if (data_type == 1) {
                for (int i = 0; i < count; ++i) { unnamed_data.integer[i] = atoi(str_data[i]); }
            } else if (data_type == 2) {
                memcpy(unnamed_data.integer, data, count * sizeof(int64_t));
            }
            break;
        case REAL:
            unnamed_data.real = (double *) calloc(count, sizeof(double));
            if (unnamed_data.real == NULL) {
                LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
                return unnamed_data;
            }

            if (data_type == 1) {
                for (int i = 0; i < count; ++i) { unnamed_data.real[i] = atof(str_data[i]); }
            } else if (data_type == 2) {
                memcpy(unnamed_data.real, data, count * sizeof(double));
            }
            break;
        case STRING:
            unnamed_data.string = (char **) calloc(count, sizeof(char *));
            if (unnamed_data.string == NULL) {
                LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
                return unnamed_data;
            }


            for (size_t i = 0; i < count; ++i) {
                size_t str_len = strlen(str_data[i]) + 1;
                unnamed_data.string[i] = (char *) calloc(str_len, sizeof(char));
                if (unnamed_data.string == NULL) {
                    LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));

                    for (size_t j = 0; j < count; ++j) { free(unnamed_data.string[j]); }
                    free(unnamed_data.string);
                    return unnamed_data;
                }

                strncpy(unnamed_data.string[i], str_data[i], str_len);
            }
            break;
        case UNDEFINED:
            break;
    }

    return unnamed_data;
}

/**
 * @brief Destroy config variable data
 *
 * @param type
 * @param data
 * @param count - use only for strings
 */
static void destroy_var_data(const ConfigVarType type, ConfigData data, const size_t count) {
    switch (type) {
        case INTEGER:
            free(data.integer);
            break;
        case REAL:
            free(data.real);
            break;
        case STRING:
            for (size_t i = 0; i < count; ++i) { free(data.string[i]); }
            free(data.string);
            break;
        case UNDEFINED:
            break;
    }
}

/**
 * @brief Create a config variable object (without data)
 *
 * @param count_items
 * @param type
 * @param name
 * @param description
 * @return ConfigVariable*
 */
static ConfigVariable *create_variable(const int count_items, const ConfigVarType type, const char *name,
                                       const char *description) {
    ConfigVariable *var = (ConfigVariable *) calloc(1, sizeof(ConfigVariable));

    var->count = count_items;
    var->type = type;

    size_t len = strlen(name) + 1;
    var->name = (char *) calloc(len, sizeof(char));
    if (var->name == NULL) {
        free(var);
        LOG(LOG_ERROR, "Can not allocate memory for define variable: %s\n", strerror(errno));
        return NULL;
    }
    strncpy(var->name, name, len);

    len = strlen(description) + 1;
    var->description = (char *) calloc(len, sizeof(char));
    if (var->description == NULL) {
        free(var->name);
        free(var);
        LOG(LOG_ERROR, "Can not allocate memory for define variable: %s\n", strerror(errno));
        return NULL;
    }
    strncpy(var->description, description, len);

    return var;
}

/**
 * @brief Destroy config variable object (without data)
 *
 * @param var
 */
static void destroy_variable(ConfigVariable *var) {
    free(var->name);
    free(var->description);
    free(var);
}

/**
 * @brief Destroy config variable object
 *
 * @param var
 */
static void destroy_variable_with_data(ConfigVariable *var) {
    destroy_var_data(var->type, var->data, const size_t count)
    free(var->name);
    free(var->description);
    free(var);
}

//=========UTILS (finite state machine)=========

/**
 * @brief This enum contains different states of parser's finite state machine
 */
typedef enum State {
    // Error states
    BAD_KEY,
    BAD_VALUE,
    MISSING_EQ_SIGN,
    VALUE_UNDEFINED,

    // Inner states
    KEY_ONLY,
    START_VALUES,
    VALUE_ARRAY,
    ARRAY_END,
    END_STRING,

    // Spaces_state
    SKIP, // this state actually defines a set of states, determined by the type from which they moved to this state

    // If config string is correct then state describes value type
    VALUE_INT,
    VALUE_REAL,
    VALUE_STRING,

    // End state
    END_VALUES,

    // Start state
    START
} State;

State transmission(State current, char input) {
    static size_t curr_key_len = 1; // '\0' - alway in key
    static bool is_array = false;
    static State state_before_skip = VALUE_UNDEFINED;

    switch (current) {
        case START:
            if (input == ' ') {
                return START;
            } else if ((input < 'A' || input > 'Z') && (input < 'a' || input > 'z') && input != '-' && input != '_'
                       || curr_key_len + 1 > MAX_CONFIG_KEY_SIZE || input == '#') {
                return BAD_KEY;
            }

            ++curr_key_len;
            return KEY_ONLY;
        // key states
        case KEY_ONLY:
            if (input == ' ') {
                state_before_skip = KEY_ONLY;
                return SKIP;
            } else if ((input < 'A' || input > 'Z') && (input < 'a' || input > 'z') && input != '-' && input != '_'
                       || curr_key_len + 1 > MAX_CONFIG_KEY_SIZE) {
                return BAD_KEY;
            } else if (input == '=') {
                curr_key_len = 0;
                return START_VALUES;
            } else if (input == '\0' || input == '\n') {
                curr_key_len = 0;
                return MISSING_EQ_SIGN;
            }

            ++curr_key_len;
            return KEY_ONLY;
        // values states
        case START_VALUES:
            if (input == ' ') {
                state_before_skip = START_VALUES;
                return SKIP;
            } else if (input == '"') {
                return VALUE_STRING;
            } else if (input >= '0' && input <= '9') {
                return VALUE_INT;
            } else if (input == '\0' || input == '\n') {
                return VALUE_UNDEFINED;
            } else if (input == '[') {
                is_array = true;
                return VALUE_ARRAY;
            }

            return BAD_VALUE;
        case VALUE_INT:
            if (input == ' ') {
                state_before_skip = VALUE_INT;
                return SKIP;
            } else if (input >= '0' && input <= '9') {
                return VALUE_INT;
            } else if (input == '.') {
                return VALUE_REAL;
            } else if (input == '\0' || input == '\n' || input == '#' && !is_array) {
                return END_VALUES;
            } else if (input == ']' && is_array) {
                return ARRAY_END;
            } else if (input == ',' && is_array) {
                return VALUE_ARRAY;
            }

            return BAD_VALUE;
        case VALUE_REAL:
            if (input == ' ') {
                state_before_skip = VALUE_REAL;
                return SKIP;
            } else if (input >= '0' && input <= '9') {
                return VALUE_REAL;
            } else if (input == '\0' || input == '\n' || input == '#' && !is_array) {
                return END_VALUES;
            } else if (input == ']' && is_array) {
                return ARRAY_END;
            } else if (input == ',' && is_array) {
                return VALUE_ARRAY;
            }

            return BAD_VALUE;
        case VALUE_STRING:
            if (input == '"') { return END_STRING; }

            return VALUE_STRING;
        case END_STRING:
            if (input == ' ') {
                return END_STRING;
            } else if (input == '\0' || input == '\n' || input == '#' && !is_array) {
                return END_VALUES;
            } else if (input == ']' && is_array) {
                return ARRAY_END;
            } else if (input == ',' && is_array) {
                return VALUE_ARRAY;
            }

            return BAD_VALUE;
        // array of values states
        case VALUE_ARRAY:
            if (input == ' ') {
                return VALUE_ARRAY;
            } else if (input == '"') {
                return VALUE_STRING;
            } else if (input >= '0' && input <= '9') {
                return VALUE_INT;
            } else if (input == '\0' || input == '\n') {
                return VALUE_UNDEFINED;
            } else if (input == ']') {
                return ARRAY_END;
            }

            return BAD_VALUE;
        case ARRAY_END:
            if (input == ' ') {
                return ARRAY_END;
            } else if (input == '\0' || input == '\n' || input == '#') {
                return END_VALUES;
            }

            return BAD_VALUE;
        case BAD_VALUE:
            return BAD_VALUE;
        case BAD_KEY:
            return BAD_KEY;
        case END_VALUES:
            return END_VALUES;
        case MISSING_EQ_SIGN:
            return MISSING_EQ_SIGN;
        case VALUE_UNDEFINED:
            return VALUE_UNDEFINED;
        case SKIP:
            if (input == ' ' || input == '\t') {
                return SKIP;
            } else if (state_before_skip == KEY_ONLY) {
                if (input == '=') {
                    return START_VALUES;
                } else {
                    return BAD_KEY;
                }
            } else if (state_before_skip == START_VALUES) {
                return START_VALUES;
            } else if (state_before_skip == VALUE_INT || state_before_skip == VALUE_REAL) {
                if (input == '\0' || input == '\n' || input == '#' && !is_array) {
                    return END_VALUES;
                } else if (input == ']' && is_array) {
                    return ARRAY_END;
                } else if (input == ',' && is_array) {
                    return VALUE_ARRAY;
                }

                return BAD_VALUE;
            }

            return VALUE_UNDEFINED;
    }
}

/**
 * @brief Analyze config raw string: extract from it key and value,
 * check if string is invalid, discards comments or empty strings and convert it to convenient from
 * @param [in] conf_str - Raw config string
 * @return Parsed config string if success or NULL if some error occurred
 * @note Memory for string interpretation and data in it will be allocated automatically, so after use you should free
 * it
 */
ConfigString *convert_config_string(const char *conf_str) {
    ConfigString *parsed_conf_str = (ConfigString *) calloc(1, sizeof(ConfigString));
    if (parsed_conf_str == NULL) {
        LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
        return NULL;
    }

    parsed_conf_str->type = UNDEFINED;
    parsed_conf_str->count = 0;

    char **array_of_data = (char **) calloc(1, sizeof(char *)); // array of byte arrays
    if (array_of_data == NULL) {
        free(parsed_conf_str);
        LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
        return NULL;
    }

    const size_t base_len = strlen(conf_str) + 1;
    array_of_data[0] = (char *) calloc(base_len, sizeof(char)); // byte array
    if (array_of_data[0] == NULL) {
        LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
        goto ERROR_EXIT;
    }

    State curr_state = START;
    int data_byte_index = 0;
    int data_index = 0;
    for (int i = 0, k = 1; conf_str[k - 1] != '\0'; ++i, ++k) {
        curr_state = transmission(curr_state, conf_str[i]);
        if (curr_state == VALUE_INT || curr_state == VALUE_REAL || curr_state == VALUE_STRING
            || curr_state == KEY_ONLY) {
            array_of_data[data_index][data_byte_index] = conf_str[i];
            ++data_byte_index;
        }

        if (curr_state == START_VALUES) {
            // save key
            memcpy(parsed_conf_str->key, array_of_data[data_index], data_byte_index + 1);
            parsed_conf_str->key[data_byte_index + 1] = '\0';

            memset(array_of_data[data_index], '\0', base_len);
            data_byte_index = 0; // found value and start rewrite it
            parsed_conf_str->type = UNDEFINED;
        } else if (curr_state == VALUE_INT) {
            parsed_conf_str->type = INTEGER;
        } else if (curr_state == VALUE_REAL) {
            parsed_conf_str->type = REAL;
        } else if (curr_state == VALUE_STRING) {
            parsed_conf_str->type = STRING;
        } else if (curr_state == VALUE_ARRAY) {
            parsed_conf_str->count++;
            data_index++;

            array_of_data = (char **) realloc(array_of_data, data_index + 1);
            if (array_of_data == NULL) {
                LOG(LOG_ERROR, "Error while reallocating memory: %s\n", strerror(errno));
                goto ERROR_EXIT;
            }

            array_of_data[data_index] = (char *) calloc(base_len, sizeof(char));
            if (array_of_data[parsed_conf_str->count] == NULL) {
                LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
                goto ERROR_EXIT;
            }
            array_of_data[data_index][data_byte_index + 1] = '\0';
            data_byte_index = 0;
        } else if (curr_state == BAD_KEY || curr_state == BAD_VALUE || curr_state == MISSING_EQ_SIGN
                   || curr_state == VALUE_UNDEFINED || curr_state == END_VALUES) {
            break;
        }
    }

    array_of_data[data_index][data_byte_index + 1] = '\0';

    switch (curr_state) {
        case END_VALUES:
            array_of_data[data_index][data_byte_index] = '\0'; // rewrite '\n'
            parsed_conf_str->count++;
            parsed_conf_str->data = create_var_data(parsed_conf_str->type, array_of_data, 1, parsed_conf_str->count);
            if (!is_data_exists(parsed_conf_str->type, parsed_conf_str->data)) {
                LOG(LOG_ERROR, "Con not create variable data. String: %s\n", array_of_data[data_index]);
                goto ERROR_EXIT;
            }
        case BAD_KEY:
            LOG(LOG_ERROR, "Invalid key: %s\n", array_of_data[data_index]);
            goto ERROR_EXIT;
        case BAD_VALUE:
            LOG(LOG_ERROR, "Invalid value on key: %s\n", array_of_data[data_index]);
            goto ERROR_EXIT;
        case MISSING_EQ_SIGN:
            LOG(LOG_ERROR, "Missing equal sign after key: %s\n", array_of_data[data_index]);
            goto ERROR_EXIT;
        default:
            LOG(LOG_ERROR, "Unkown error in string: %s...\n", array_of_data[data_index]);
            goto ERROR_EXIT;
    }

    for (int i = 0; i < data_index + 1; ++i) { free(array_of_data[data_index]); }
    free(array_of_data);
    return parsed_conf_str;

ERROR_EXIT:
    free(parsed_conf_str);

    for (int i = 0; i < data_index + 1; ++i) { free(array_of_data[data_index]); }
    free(array_of_data);

    return NULL;
}

//=========UTILS (config reader)=========

/**
 * @brief Checks whether reading the next character will result in the EOF.
 *  If EOF has not been reached move file position indicator back one character
 *
 * @param config - pointer to config file
 * @return true - end of file has been reached or some error occurred
 * @return false - in another case
 */
static bool is_eof(FILE *config) {
    char character = fgetc(config);
    if (character == EOF) return true;

    if (fseek(config, -1, SEEK_CUR) < 0) {
        LOG(LOG_ERROR, "Can not move file position indicator. File stream now invalid! Fseek error: %s\n",
            strerror(errno));

        return true;
    }
    return false;
}

/**
 * @brief Buffered reader. Read full config string. If buffer size not enough then realloc it
 *
 * @param config - pointer to config file
 * @param conf_raw_string - pointer to config string
 * @param size - start size of the config string
 * @return true - if config string reading successfully
 * @return false - if some error occurred or EOF has been reached (if error occurred conf_raw_string will be NULL)
 */
static bool read_config_string(FILE *config, char **conf_raw_string, size_t size) {
    while (fgets(*conf_raw_string, size, config) != NULL) {
        if ((*conf_raw_string)[strlen(*conf_raw_string) - 1] != '\n') {
            // If read last string (it doesn`t contains '\n') return true for analyze it
            // If this check is not done then looping will occur (because fseek shift our position by number of reading
            // bytes)
            if (is_eof(config)) return true;

            if (fseek(config, (-1) * strlen(*conf_raw_string), SEEK_CUR) < 0) {
                LOG(LOG_ERROR, "Can not read config string. Fseek error: %s\n", strerror(errno));
                return false;
            }
            *conf_raw_string = (char *) realloc(*conf_raw_string, size * 2);
            if (conf_raw_string == NULL) {
                LOG(LOG_ERROR, "Can not read config string. Realloc error: %s\n", strerror(errno));
                return false;
            }

            size *= 2;
            continue;
        } else
            (*conf_raw_string)[strlen(*conf_raw_string) - 1] = '\0';

        return true;
    }

    // if NULL was returned because EOF was reached
    if (feof(config)) return true;
    // else check errors
    else if (ferror(config))
        LOG(LOG_ERROR, "Error occurred while reading config file: %s", strerror(errno));

    return false;
}


//=========CONFIG FUNCTIONS=========

/**
 *  Get variable from config.
 *  @param [in] name name of variable
 *  @return ConfigVariable struct if all is OK and ConfigVariable struct with
 * type UNDEFINED if some error occurred
 */
ConfigVariable get_variable(const char *name) {
    ConfigVariable *var = get_map_element(map, name);
    if (var == NULL) {
        ConfigVariable undef_var
                = {.name = UNDEF_VAR_NAME, .description = UNDEF_VAR_DESCR, .data = NULL, .count = 0, .type = UNDEFINED};
        return undef_var;
    }

    return *var;
}

/**
 * @brief Determine if variable exists in config
 * @param [in] name name of variable
 * @return true if variable exists and false if not
*/
bool does_variable_exist(const char *name) {
    ConfigVariable *var = (ConfigVariable *) get_map_element(map, name);

    if (var == NULL) return false;

    return true;
}

/**
 * @brief Parse configuration file located on path.
 * @param [in] path path to configuration file. It must not be NULL.
 * @return -1 if some error occurred or 0 if all is OK.
 */
int parse_config(const char *path) {
    if (path == NULL) { return -1; }

    FILE *config = fopen(path, "r");
    if (config == NULL) {
        LOG(LOG_ERROR, "Can`t read config file: %s. Use default config values\n", strerror(errno));
        return -1;
    }

    char *conf_raw_string = (char *) calloc(MAX_CONFIG_KEY_SIZE + START_CONFIG_VALUE_SIZE + 2, sizeof(char));
    if (conf_raw_string == NULL) {
        fclose(config);
        LOG(LOG_ERROR, "Can`t allocate memory: %s\n", strerror(errno));
        return -1;
    }

    ConfigVariable *var = NULL;

    ConfigString *conf_string = NULL;

    // We read the line up to the line break character and read it separately so as not to interfere
    while (read_config_string(config, &conf_raw_string, MAX_CONFIG_KEY_SIZE + START_CONFIG_VALUE_SIZE + 2)) {
        // get key and value from config string. This function also returns buffer with data
        conf_string = convert_config_string(conf_raw_string);
        if (conf_string == NULL || does_variable_exist(conf_string->key)) {
            destroy_var_data(conf_string->type, conf_string->data, conf_string->count);
            free(conf_string);
            continue;
        }

        var = create_variable(conf_string->count, conf_string->type, conf_string->key, STANDARD_DESCRIPTION);
        if (var == NULL) {
            LOG(LOG_ERROR, "Can not create config file defined variable: %s\n", conf_string->key);
            destroy_variable(var);
            destroy_var_data(conf_string->type, conf_string->data, conf_string->count);
            free(conf_string);
            continue;
        }
        var->data = conf_string->data; // data allocated in convert_config_string so we can only move pointer to it

        // push_to_map_copy make copy of variable (but not the data pointed to by the pointer inside the variable)
        if (push_to_map(map, conf_string->key, var) < 0) {
            LOG(LOG_ERROR,
                "Error occurred while adding new variable from config file. Continue without user value for variable: "
                "%s\n",
                var->name);
            destroy_variable(var);
            destroy_var_data(conf_string->type, conf_string->data, conf_string->count);
            free(conf_string);
            continue;
        }

        // we free only conf_string without data buffer because it will be used in config throughout his work
        free(conf_string);
    }

    free(conf_raw_string);
    fclose(config);

    return 0;
}

/**
 *  Define variable from code.
 *  @param [in] variable ConfigVariable struct with initial values
 *  @return -1 if variable already exists,
 *          -2 if some error occurred,
 *          0 if variable created successfully
 */
int define_variable(const ConfigVariable variable) {
    if (does_variable_exist(variable.name)) { return -1; }

    ConfigVariable *pushed_var = create_variable(variable.count, variable.type, variable.name, variable.description);
    if (pushed_var == NULL) {
        LOG(LOG_ERROR, "Can not create user defined variable: %s\n", variable.name);
        return -2;
    }

    ConfigData data;

    pushed_var->data = data;

    if (push_to_map(map, pushed_var->name, pushed_var) < 0) {
        destroy_variable(pushed_var);
        destroy_var_data(variable.type, data, variable.count);
        return -2;
    }

    return 0;
}

/**
 *  Define variable if does not exists or set it if it already exists.
 *  @param [in] variable ConfigVariable struct with initial values
 *  @return -1 if some error occurred or 0 if all is OK
*/
int set_variable(const ConfigVariable variable) {
    ConfigVariable *var = (ConfigVariable *) get_map_element(map, variable.name);

    if (var == NULL) {
        if (define_variable(variable) < 0) {
            return -1;
        }
        return 0;
    }

    
    ConfigData data;
    switch (variable.type) {
        case UNDEFINED:
            LOG(LOG_WARNING, "Try to define variable with udefined type\n");
            return -1;
        case INTEGER:
            data = create_var_data(variable.type, variable.data.integer, 2, variable.count);
            if (!is_data_exists(variable.type, data)) {
                LOG(LOG_ERROR, "Can not create user defined variable - %s - and data for it\n", variable.name);
                return -1;
            }
            break;
        case REAL:
            data = create_var_data(variable.type, variable.data.real, 2, variable.count);
            if (!is_data_exists(variable.type, data)) {
                LOG(LOG_ERROR, "Can not create user defined variable - %s - and data for it\n", variable.name);
                return -1;
            }
            break;
        case STRING:
            data = create_var_data(variable.type, variable.data.string, 2, variable.count);
            if (!is_data_exists(variable.type, data)) {
                LOG(LOG_ERROR, "Can not create user defined variable - %s - and data for it\n", variable.name);
                return -1;
            }
            break;
    }

    ConfigData old_data = var->data;
    destroy_var_data(var->type, old_data, var->count);
    var->data = data;
    var->count = variable.count;

    return 0;
}

/**
 * @brief Create a config table object
 *
 * @return -1 if table already exists or 0 if all is OK
 */
int create_config_table(void) {
    if (is_config_created) { return -1; }

    map = create_map();
    if (map == NULL) {
        LOG(LOG_ERROR, "Can not allocate memory for config map\n");
        return -1;
    }

    is_config_created = true;
    return 0;
}

/**
 *  Destroy config table and frees all resources associated with it. It should
 *  be called once.
 *  @return -1 if table already destroyed or 0 if all is OK
*/
int destroy_config_table(void) {
    if (is_config_created) {
        is_config_created = false;
        destroy_map_with_data(map, void (*data_destroyer)(void *))
        return 0;
    }

    return -1;
}


int destroy_guc_table() {
    destroy_map(&map);

    return 0;
}
