#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "../../include/master.h"

/*HOOKS DEFINITIONS*/
Hook executor_start_hook = NULL;
Hook executor_end_hook = NULL;

/*STRUCTS*/


/*FUNCTION DECLARATIONS*/

/*!
    \struct Plugin
    \brief Pair of dlopen handle and name
*/
struct Plugin {
    void *handle;
    char *name;
};

/*!
    \struct PluginsStack
    \brief Stack for plugin pointers
*/
struct PluginsStack {
    struct Plugin *plugins;
    int size;
    int max_idx;
};

/*FUNCTION DEFINITIONS*/

/*!
    \brief Initializes stack for plugins
    \param[in] bootSize Start size of stack
    \return Pointer to new stack
*/
struct PluginsStack* init_plugins_stack(int boot_size)
{
    struct PluginsStack* stack = (struct PluginsStack*)malloc(
        sizeof(struct PluginsStack));
    stack->max_idx = -1;
    stack->size = 0;
    stack->plugins = (struct Plugin*)calloc(boot_size, sizeof(struct Plugin));
    return stack;
}


/*!
    \brief Finishes all plugins and closes handles
    \param[in] stack Pointer to stack
    \return 0 if success, -1 and sets -1 else;
*/
void close_plugins(struct PluginsStack* stack) {
    void (*fini_plugin)(void) = NULL;
    for (int i = stack->max_idx; i >= 0; i--)
    {
        fini_plugin = (void(*)(void))dlsym(stack->plugins[i].handle, "fini");
        fini_plugin();
        dlclose(stack->plugins[i].handle);
    }
}

/*!
    \brief Pushes plugin to stack
    \param[in] stack Pointer to stack
    \param[in] plugin Plugin's pointer
    \param[in] name Plugin's name
    \return 0 if success, -1 else
*/
int push_plugin(struct PluginsStack* stack, void* plugin, char* name)
{
    if (stack->max_idx == stack->size - 1)
    {
        stack->size = (int)(stack->size * 1.1 + 1);
        stack->plugins = (struct Plugin*)realloc(stack->plugins,
                                                 stack->size * sizeof(struct
                                                     Plugin));
    }
    stack->plugins[++stack->max_idx].handle = plugin;
    stack->plugins[stack->max_idx].name = name;

    return 0;
}

/*!
    \brief pop plugin from stack
    Pops plugin from stack
    \param[in] stack Pointer to stack
    \return plugin if success, NULL else
*/
struct Plugin pop_plugin(struct PluginsStack* stack)
{
    struct Plugin result;
    result.handle = NULL;
    result.name = NULL;
    if (stack->size == 0)
    {
        return result;
    }
    if (!stack->plugins[stack->max_idx].handle)
    {
        return result;
    }
    return stack->plugins[stack->max_idx--];
}

/*!
    \brief gets plugin by it's name
    \param[in] stack Pointer to plugin stack
    \param[in] name Name of plugin
    \return plugin if success, empty plugin structure else
*/
struct Plugin get_plugin(struct PluginsStack* stack, char* name)
{
    struct Plugin result;
    result.handle = NULL;
    result.name = NULL;
    for (int i = 0; i < stack->size; i++)
    {
        if (strcmp(name, stack->plugins[i].name) == 0)
        {
            return stack->plugins[i];
        }
    }
    return result;
}

/*!
    \brief creates path to plugin
    \param[in] fileName Name of dynamic library
    \param[in] pluginsDir Path to plugins directory (could be NULL)
    \return 0 if success, -1 else
*/
char* mk_plugin_path(const char* file_name, const char* plugins_dir)
{
    if (!file_name)
    {
        return NULL;
    }

    if (!plugins_dir)
    {
        plugins_dir = "./plugins/";
    }

    char* slash = "";
    if (plugins_dir[strlen(plugins_dir) - 1] != '/')
    {
        slash = "/";
    }

    char* plugin_path = (char*)calloc(
        strlen(plugins_dir) + strlen(slash) + strlen(file_name) + strlen(".so") + 1,
        sizeof(char));
    return strcat(strcat(strcat(strcat(plugin_path, plugins_dir), slash), file_name),
                  ".so");
}

/*!
    \brief Loads plugins
    \param[in] plugins_list array of plugin names
    \param[in] plugins_count array size
    \param[in] pluginsDir plugins directory (could be NULL)
    \param[in] stack Stack for plugins
    \return 0 if success, -1 else
*/
int load_plugins(char **plugins_list, int plugins_count, char* plugins_dir,
                struct PluginsStack* stack)
{
    void (*init_plugin)(void);
    ///< this function will be executed for each contrib library
    void* handle;
    char* error;

    for (int i = 0; (i < plugins_count); ++i)
    {
        /*TRY TO LOAD .SO FILE*/

        char* plugin_path = mk_plugin_path(plugins_list[i], plugins_dir);
        handle = dlopen(plugin_path, RTLD_NOW | RTLD_GLOBAL);

        if (!handle)
        {
            error = dlerror();
            fprintf(
                stderr,
                "Library couldn't be opened.\n\tLibrary's path is %s\n\tdlopen: %s\n\tcheck plugins folder or rename library\n",
                plugin_path, error);
            free(stack);
            return -1;
        }
        error = dlerror();

        /*PUSH HANDLE TO PLUGIN STACK*/
        push_plugin(stack, handle, plugins_list[i]);

        /*EXECUTE PLUGIN*/
        init_plugin = (void(*)(void))dlsym(handle, "init");

        error = dlerror();
        if (error != NULL)
        {
            fprintf(
                stderr,
                "Library couldn't execute init.\n\tLibrary's name is %s. Dlsym message: %s\n\tcheck plugins folder or rename library",
                plugins_list[i], error);
            free(stack);
            return -1;
        }

        init_plugin();
    }
    return 0;
}


int main(int argc, char **argv) {
    struct PluginsStack *plugins = init_plugins_stack(100);

    printf("%s\n", argv[0]);
    char *greeting_name = "greeting";
    char **plugin_names = &greeting_name;

    load_plugins(plugin_names, 1, NULL, plugins);

    if (executor_start_hook)
        executor_start_hook();

    close_plugins(plugins);
    free(plugins);

    return 0;
}