/*!
    Prepares plugin for work. Links all hooks.
*/
void init(void);

/*!
    Frees all resources occupied by plugin. Unlinks all hooks.
*/
void fini(void);

/*!
    \return name of plugin as string constant 
*/
const char *name(void);