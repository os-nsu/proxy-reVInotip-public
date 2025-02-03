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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/logger.h"
#include "../../include/utils/hash_map.h"
#include "../../include/utils/stack.h"

#define MAX_CONFIG_KEY_SIZE 128
#define START_CONFIG_VALUE_SIZE 200

static HashMapPtr map;
static StackPtr alloc_mem_stack;

bool is_config_created = false;

typedef struct ConfigString {
    char key[MAX_CONFIG_KEY_SIZE];
    ConfigVariable *value;
} ConfigString;

//=========UTILS (finite state machine)=========

/**
 * @brief This enum contains different states of parser's finite state machine
 */
typedef enum State {
    // Error states
    BAD_KEY,
    BAD_VALUE,
    MISSING_EQ_SIGN,

    // Inner states
    NEXT_VALUE,
    KEY_ONLY,
    VALUE_UNDEFINED,

    // If config string is correct then state describes value type
    VALUE_INT,
    VALUE_REAL,
    VALUE_STRING,

    // End states
    END_STRING,
    END_VALUES,

    // Start state
    START
} State;

State transmission(State current, char input) {
    static size_t curr_key_len = 0;
    if (input == ' ') { return current; }

    switch (current) {
        case START:
            if ((input < 'A' || input > 'Z') && (input < 'a' || input > 'z') && input != '-' && input != '_'
                || curr_key_len + 1 > MAX_CONFIG_KEY_SIZE || input == '#') {
                return BAD_KEY;
            }

            ++curr_key_len;
            return KEY_ONLY;
        case KEY_ONLY:
            if ((input < 'A' || input > 'Z') && (input < 'a' || input > 'z') && input != '-' && input != '_'
                || curr_key_len + 1 > MAX_CONFIG_KEY_SIZE) {
                return BAD_KEY;
            } else if (input == '=') {
                curr_key_len = 0;
                return VALUE_UNDEFINED;
            } else if (input == '\0' || input == '\n') {
                curr_key_len = 0;
                return MISSING_EQ_SIGN;
            }

            ++curr_key_len;
            return KEY_ONLY;
        case VALUE_UNDEFINED:
            if (input == '"') {
                return VALUE_STRING;
            } else if (input >= '0' && input <= '9') {
                return VALUE_INT;
            } else if (input == '[') {
                return NEXT_VALUE;
            } else if (input == '\0' || input == '\n') {
                return VALUE_UNDEFINED;
            }

            return BAD_VALUE;
        case VALUE_INT:
            if (input >= '0' && input <= '9') {
                return VALUE_INT;
            } else if (input == '.') {
                return VALUE_REAL;
            } else if (input == '\0' || input == '\n' || input == '#') {
                return END_VALUES;
            } else if (input == ',') {
                return NEXT_VALUE;
            }

            return BAD_VALUE;
        case VALUE_REAL:
            if (input >= '0' && input <= '9') {
                return VALUE_REAL;
            } else if (input == '\0' || input == '\n' || input == '#') {
                return END_VALUES;
            } else if (input == ',') {
                return NEXT_VALUE;
            }

            return BAD_VALUE;
        case VALUE_STRING:
            if (input == '"') { return END_STRING; }

            return VALUE_STRING;
        case NEXT_VALUE:
            if (input == '"') {
                return VALUE_STRING;
            } else if (input >= '0' && input <= '9') {
                return VALUE_INT;
            } else if (input == ']') {
                return END_VALUES;
            } else if (input == '\0' || input == '\n') {
                return BAD_VALUE;
            }

            return BAD_VALUE;
        case BAD_VALUE:
            return BAD_VALUE;
        case BAD_KEY:
            return BAD_KEY;
        case END_STRING:
            if (input == '\0' || input == '\n' || input == '#') {
                return END_VALUES;
            } else if (input == ',') {
                return NEXT_VALUE;
            }

            return BAD_VALUE;
        case END_VALUES:
            return END_VALUES;
        case MISSING_EQ_SIGN:
            return MISSING_EQ_SIGN;
    }
}

/**
 * @brief Analyze config raw string: extract from it key and value,
 * check if string is invalid, discards comments or empty strings and convert it to convenient from
 * @param [in] conf_str - Raw config string
 * @return Parsed config string if success or NULL if some error occurred
 * @note Memory for string interpretation will be allocated automatically, so after use you should free it
 * @note Memory for config variable also will be allocated automatically
 */
ConfigString *convert_config_string(const char *conf_str) {
    ConfigVariable *variable = (ConfigVariable *) calloc(1, sizeof(ConfigVariable));
    if (variable == NULL) {
        LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
        return NULL;
    }

    variable->type = UNDEFINED;
    variable->count = 0;
    variable->description = NULL;
    variable->name = NULL;

    ConfigString *parsed_conf_str = (ConfigString *) calloc(1, sizeof(ConfigString));
    if (parsed_conf_str == NULL) {
        free(variable);
        LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
        return NULL;
    }

    parsed_conf_str->value = variable;

    char *data = (char *) calloc(strlen(conf_str) + 1, sizeof(char));
    if (data == NULL) {
        free(variable);
        free(parsed_conf_str);
        LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
        return NULL;
    }

    State curr_state = START;
    int j = 0;
    for (int i = 0, k = 1; conf_str[k - 1] != '\0'; ++i, ++j, ++k) {
        curr_state = transmission(curr_state, conf_str[i]);
        data[j] = conf_str[i];

        switch (curr_state) {
            case VALUE_UNDEFINED:
                data[j + 1] = '\0';
                strcpy(parsed_conf_str->key, data);

                memset(data, '\0', strlen(conf_str) + 1);
                j = 0; // found value and start rewrite it
                parsed_conf_str->value->type = UNDEFINED;
            case VALUE_INT:
                parsed_conf_str->value->type = INTEGER;
            case VALUE_REAL:
                parsed_conf_str->value->type = REAL;
            case VALUE_STRING:
                parsed_conf_str->value->type = STRING;
            case NEXT_VALUE:
                parsed_conf_str->value->count++;
            case END_STRING:
                continue;
            case KEY_ONLY:
                continue;
            default:
                break;
        }
    }

    switch (curr_state) {
        case END_VALUES:
            data[j] = '\0';
            parsed_conf_str->value->count++;
            switch (parsed_conf_str->value->type) {
                case INTEGER:
                    parsed_conf_str->value->data.integer = (int64_t *) calloc(parsed_conf_str->value->count, sizeof(int64_t));
                    for (int i = 0; i < parsed_conf_str->value->count; ++i) {
                        parsed_conf_str->value->data.integer[i] = atoi(data);
                    }
                case REAL:
                    parsed_conf_str->value->data.real = (double *) calloc(parsed_conf_str->value->count, sizeof(double));
                    for (int i = 0; i < parsed_conf_str->value->count; ++i) {
                        parsed_conf_str->value->data.real[i] = atof(data);
                    }
                case STRING:
                    *parsed_conf_str->value->data.string = (char *) calloc(strlen(data) + 1, sizeof(char));
                    if (parsed_conf_str->value->data.string == NULL) {
                        LOG(LOG_ERROR, "Error while allocating memory: %s\n", strerror(errno));
                        goto ERROR_EXIT;
                    }

                    strcpy(*parsed_conf_str->value->data.string, data);
                case UNDEFINED:
                    LOG(LOG_ERROR, "Can not determine type of variable: %s\n", parsed_conf_str->key);
                    goto ERROR_EXIT;
            }
        case BAD_KEY:
            data[j + 1] = '\0';
            LOG(LOG_ERROR, "Invalid key: %s\n", data);
            goto ERROR_EXIT;
        case BAD_VALUE:
            LOG(LOG_ERROR, "Invalid value on key: %s\n", data);
            goto ERROR_EXIT;
        case MISSING_EQ_SIGN:
            data[j + 1] = '\0';
            LOG(LOG_ERROR, "Missing equal sign after key: %s\n", data);
            goto ERROR_EXIT;
        default:
            data[j + 1] = '\0';
            LOG(LOG_ERROR, "Unkown error in string: %s...\n", data);
            goto ERROR_EXIT;
    }

    free(data);
    return parsed_conf_str;

ERROR_EXIT:
    free(parsed_conf_str);
    free(variable);
    free(data);

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
bool is_eof(FILE *config) {
    char c = fgetc(config);
    if (c == EOF) return true;

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
bool read_config_string(FILE *config, char **conf_raw_string, size_t size) {
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
    if (feof(config))
        return true;
    // else check errors
    else if (ferror(config))
        LOG(LOG_ERROR, "Error occurred while reading config file: %s", strerror(errno));

    return false;
}

//=========CONFIG FUNCTIONS=========

void create_default_conf_vars(void) {
    // create vars
}

/**
 * @brief Determine if variable exists in config
 * @param [in] name name of variable
 * @return true if variable exists and false if not
 REWRITE
*/
bool does_variable_exist(const char* name); {
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);

    if (var == NULL || !equals(get_identify(var->context), get_identify(context))) return false;

    return true;
}

/**
 * @brief Parse configuration file located on path.
 * @param [in] path path to configuration file. It must not be NULL.
 * @return -1 if some error occurred or 0 if all is OK.
*/
int parse_config(const char* path) {
    if (path == NULL) { return -1; }

    FILE *config = fopen(path, "r");
    if (config == NULL) {
        LOG(LOG_ERROR, "Can`t read config file: %s. Use default config values\n", strerror(errno));
        create_default_conf_vars();
        return -1;
    }

    alloc_mem_stack = create_stack();

    char *conf_raw_string = (char *) malloc(MAX_CONFIG_KEY_SIZE + START_CONFIG_VALUE_SIZE + 2);
    ConfigVariable var;
    ConfigString* conf_string = NULL;
    //conf_string.value = &(var); // add pointer for auto fill var variable

    // We read the line up to the line break character and read it separately so as not to interfere
    while (read_config_string(config, &conf_raw_string, MAX_CONFIG_KEY_SIZE + START_CONFIG_VALUE_SIZE + 2)) {
        memset(conf_string.key, 0, MAX_CONFIG_KEY_SIZE);

        // get key and value from config string
        conf_string = convert_config_string(conf_raw_string);
        if (conf_string == NULL || does_variable_exist(conf_string->key)) continue;

        if (type == ARR_LONG) {
            push_to_stack(&alloc_mem_stack, var.elem.arr_long.data);
            var.elem.arr_long.count_elements = conf_string.count_values;
        } else if (type == ARR_DOUBLE) {
            push_to_stack(&alloc_mem_stack, var.elem.arr_double.data);
            var.elem.arr_double.count_elements = conf_string.count_values;
        } else if (type == ARR_STRING) {
            push_to_stack(&alloc_mem_stack, var.elem.arr_str.data);
            uint64_t counter = conf_string.count_values;
            while (counter != 0) {
                --counter;
                push_to_stack(&alloc_mem_stack, var.elem.arr_str.data[counter]);
            }
            var.elem.arr_str.count_elements = conf_string.count_values;
        } else if (type == STRING)
            push_to_stack(&alloc_mem_stack, var.elem.str);

        var.vartype = type;

        // all config varibles is immutable
        var.context = C_MAIN | C_STATIC;
        strcpy(var.name, conf_string.key);
        strcpy(var.descripton, STANDART_DESCRIPTION);

        push_to_map(map, conf_string.key, &var, sizeof(Guc_variable));
    }

    free(conf_raw_string);

    if (fclose(config) == EOF) {
        write_stderr("Can not close configuration file: %s", strerror(errno));
        return;
    }
}

/**
 * \brief Define your own GUC long varible
 * \param [in] name - name of new variable
 * \param [in] descr - description of new variable
 * \param [in] boot_value - initial value of variable
 * \param [in] context - restriction on variable use
 * \return nothing
 */
void define_custom_long_variable(char *name, const char *descr, const long boot_value, const int8_t context) {
    Guc_variable var;
    strcpy(var.name, name);
    strcpy(var.descripton, descr);
    var.elem.num = boot_value;
    var.context = context;
    var.vartype = LONG;

    push_to_map(map, name, &var, sizeof(Guc_variable));
}

/**
 * \brief Define your own GUC string varible
 * \param [in] name - name of new variable
 * \param [in] descr - description of new variable
 * \param [in] boot_value - initial value of variable
 * \param [in] context - restriction on variable use
 * \return nothing
 */
void define_custom_string_variable(char *name, const char *descr, const char *boot_value, const int8_t context) {
    Guc_variable var;
    strcpy(var.name, name);
    strcpy(var.descripton, descr);
    var.elem.str = (char *) malloc(strlen(boot_value) + 1);
    strcpy(var.elem.str, boot_value);
    var.context = context;
    var.vartype = STRING;

    push_to_map(map, name, &var, sizeof(Guc_variable));
    push_to_stack(&alloc_mem_stack, var.elem.str);
}

/**
 * \brief Get GUC variable by its name
 */
Guc_data get_config_parameter(const char *name, const int8_t context) {
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);
    Guc_data err_data = {0};

    if (var == NULL) {
        write_stderr("Config variable %s does not exists\n", name);
        return err_data;
    }

    if (!equals(get_identify(var->context), get_identify(context))) {
        elog(ERROR, "Try to get variable with different context");
        return err_data;
    }

    return var->elem;
}

/**
 * \brief Get GUC variable by its name
 */
Config_vartype get_config_parameter_type(const char *name, const int8_t context) {
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);

    if (var == NULL) {
        write_stderr("Config variable %s does not exists\n", name);
        return UNINIT;
    }

    if (!equals(get_identify(var->context), get_identify(context))) {
        elog(ERROR, "Try to get variable with different context");
        return UNINIT;
    }

    return var->vartype;
}

/**
 * \brief Set new value to string GUC variable (only if context allows)
 */
void set_string_config_parameter(const char *name, const char *data, const int8_t context) {
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);

    if (var == NULL) { elog(ERROR, "Config variable %s does not exists", name); }

    if (!is_dynamic(var->context)) {
        elog(ERROR, "Try to change static config variable");
        return;
    }

    if (!equals(get_identify(var->context), get_identify(context))) {
        elog(ERROR, "Try to change variable with different context");
        return;
    }

    strcpy(var->elem.str, data);
}

/**
 * \brief Set new value to long GUC variable (only if context allows)
 */
void set_long_config_parameter(const char *name, const long data, const int8_t context) {
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);

    if (var == NULL) { elog(ERROR, "Config variable %s does not exists", name); }

    if (!is_dynamic(var->context)) {
        elog(ERROR, "Try to change static config variable");
        return;
    }

    if (!equals(get_identify(var->context), get_identify(context))) {
        elog(ERROR, "Try to change variable with different context");
        return;
    }

    var->elem.num = data;
}

/**
 * @brief Create a config table object
 *
 * @return -1 if table already exists or 0 if all is OK
 */
int create_config_table(void) {
    if (is_config_created) { return -1; }

    is_config_created = true;
    return 0;
}

/*!
    Destroy config table and frees all resources associated with it. It should
   be called once.
    @return -1 if table already destroyed or 0 if all is OK
*/
int destroy_config_table(void) {
    if (is_config_created) {
        is_config_created = false;
        return 0;
    }

    return -1;
}

void create_guc_table() {
    map = create_map();
}

int destroy_guc_table() {
    while (alloc_mem_stack) {
        void *pointer = pop_from_stack(&alloc_mem_stack);
        free(pointer);
    }
    destroy_map(&map);

    return 0;
}
