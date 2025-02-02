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

#include "../../include/config.h"
#include <cstddef>
#include <cstdint>

/**
 * @brief Create a config table object
 *
 * @return -1 if table already exists or 0 if all is OK
 */
int create_config_table(void) {
    return 0;
}

/**
 * TODO
 *  Split analize_config_string function with read_config_string function
 */
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/config.h"
#include "../../include/logger.h"
#include "../../include/utils/hash_map.h"
#include "../../include/utils/stack.h"

#define MAX_CONFIG_KEY_SIZE 128

static HashMapPtr map;
static StackPtr alloc_mem_stack;

typedef struct ConfigString {
    char key[MAX_CONFIG_KEY_SIZE];
    ConfigVariable *value;
} ConfigString;

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
 * \brief Analyze config raw string: extract from it key and value,
 * check if string is invalid, discards comments or empty strings
 * \param [in] conf_str - Raw config string
 * \return Parsed config string if success or NULL if some error occurred
 * \note Memory for string interpretation will be allocated automatically, so after use you should free it
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

    switch(curr_state) {
        case END_VALUES:
            data[j] = '\0';
            parsed_conf_str->value->count++;
            switch(parsed_conf_str->value->type) {
                case INTEGER:
                    *parsed_conf_str->value->data.integer = atoi(data);
                case REAL:
                    *parsed_conf_str->value->data.real = atof(data);
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
            data[j+1] = '\0';
            LOG(LOG_ERROR, "Invalid key: %s\n", data);
            goto ERROR_EXIT;
        case BAD_VALUE:
            LOG(LOG_ERROR, "Invalid value on key: %s\n", data);
            goto ERROR_EXIT;
        case MISSING_EQ_SIGN:
            data[j+1] = '\0';
            LOG(LOG_ERROR, "Missing equal sign after key: %s\n", data);
            goto ERROR_EXIT;
        default:
            data[j+1] = '\0';
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

void destroy_guc_table() {
    while (alloc_mem_stack) {
        void *pointer = pop_from_stack(&alloc_mem_stack);
        free(pointer);
    }
    destroy_map(&map);
}

bool is_eof(FILE *config) {
    char c = fgetc(config);
    if (c == EOF) return true;

    fseek(config, -1, SEEK_CUR);
    return false;
}

bool read_config_string(FILE *config, char **conf_raw_string, size_t size) {
    while (!is_eof(config)) {
        if (fgets(*conf_raw_string, size, config) == NULL) break;

        if ((*conf_raw_string)[strlen(*conf_raw_string) - 1] != '\n') {
            // If read last string (it doesn`t contains '\n') return true for analyze it
            // If this not done then looping will occur (because fseek shift our position by number of reading
            // bytes)
            if (is_eof(config)) return true;

            fseek(config, (-1) * strlen(*conf_raw_string), SEEK_CUR);
            *conf_raw_string = (char *) realloc(*conf_raw_string, size * 2);
            size *= 2;
            continue;
        } else
            (*conf_raw_string)[strlen(*conf_raw_string) - 1] = '\0';

        return true;
    }

    if (ferror(config)) LOG(LOG_ERROR, "Error occurred while reading config file: %s", strerror(errno));

    return false;
}

void create_guc_table() {
    map = create_map();
}

/**
 * \brief Parse configuration file and create GUC variables from config variables
 * \note If variable now exists in GUC table with C_MAIN context
 *  (variables with user context can not be in table then main process call this function) it will be ignored
 */
void parse_config() {
    FILE *config;
    if (!is_var_exists_in_config("conf_path", C_MAIN | C_STATIC)) {
        printf("Use default configuration file\n");
        define_custom_string_variable("conf_path", "Path to configuration file", DEFAULT_CONF_FILE_PATH,
                                      C_MAIN | C_DYNAMIC);
        config = fopen(DEFAULT_CONF_FILE_PATH, "r");
    } else {
        Guc_data conf_path = get_config_parameter("conf_path", C_MAIN | C_DYNAMIC);
        config = fopen(conf_path.str, "r");
        if (config == NULL) {
            conf_path.str = DEFAULT_CONF_FILE_PATH;
            set_string_config_parameter("conf_path", conf_path.str, C_MAIN | C_DYNAMIC);
            write_stderr("Can`t read user config file: %s. Try to read default config file\n", strerror(errno));
            config = fopen(DEFAULT_CONF_FILE_PATH, "r");
        }
    }

    alloc_mem_stack = create_stack();

    if (config == NULL) {
        write_stderr("Can`t read config file: %s. Use default GUC values\n", strerror(errno));
        return;
    }

    char *conf_raw_string = (char *) malloc(MAX_CONFIG_KEY_SIZE + CONFIG_VALUE_SIZE + 2);
    Guc_variable var;
    Conf_string conf_string;
    conf_string.value = &(var.elem);
    bool string_is_incorrect = true;

    // We read the line up to the line break character and read it separately so as not to interfere
    while (read_config_string(config, &conf_raw_string, MAX_CONFIG_KEY_SIZE + CONFIG_VALUE_SIZE + 2)) {
        memset(conf_string.key, 0, MAX_CONFIG_KEY_SIZE);

        // get key and value from config string
        Config_vartype type = analize_config_string(conf_raw_string, &conf_string, &string_is_incorrect);

        if (string_is_incorrect || type == UNINIT || is_var_exists_in_config(conf_string.key, C_MAIN)) continue;

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
 * \brief Check if variable with name "name" exists in GUC table
 * \return true if variable with name "name" exists in GUC table and it`s context equals with param context,
 *         false if not
 */
bool is_var_exists_in_config(const char *name, const int8_t context) {
    Guc_variable *var = (Guc_variable *) get_map_element(map, name);

    if (var == NULL || !equals(get_identify(var->context), get_identify(context))) return false;

    return true;
}
