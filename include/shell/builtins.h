#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

// Define a function pointer type for built-in handlers
typedef int (*builtin_func_t)(command_t*);

typedef struct 
{
    char *name;
    builtin_func_t func;
} builtin_t;

// Function prototypes
int is_builtin(command_t *cmd);
int execute_builtin(command_t *cmd);

// Built-in command handlers
int builtin_cd(command_t *cmd);
int builtin_pwd(command_t *cmd);
int builtin_help(command_t *cmd);
int builtin_exit(command_t *cmd);

#endif