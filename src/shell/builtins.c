// To expose POSIX features like setenv, unsetenv
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../include/shell/builtins.h"
#include "../../include/shell/history.h"

// Array of built-in commands
// The {NULL, NULL} at the end is the "Sentinel" that stops the loops.
builtin_t builtins[] = 
{
    {"cd", builtin_cd},
    {"pwd", builtin_pwd},
    {"help", builtin_help},
    {"exit", builtin_exit},
    {"export", builtin_export},
    {"unset", builtin_unset},
    {"history", builtin_history},
    {NULL, NULL}
};

int is_builtin(command_t *cmd)
{
    if (!cmd || !cmd->args[0])
    {
        return 0;
    }

    for (int i = 0; builtins[i].name != NULL; i++)
    {
        if (strcmp(cmd->args[0], builtins[i].name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int execute_builtin(command_t *cmd)
{
    for (int i = 0; builtins[i].name != NULL; i++)
    {
        if (strcmp(cmd->args[0], builtins[i].name) == 0)
        {
            return builtins[i].func(cmd);
        }
    }
    return 1; // Should not reach here if `is_builtin` was checked
}

int builtin_cd(command_t *cmd)
{
    char *path = cmd->args[1];

    // If no argument is provided (just "cd"), go to HOME
    if (path == NULL)
    {
        path = getenv("HOME");
        if (path == NULL)
        {
            fprintf(stderr, "unixsh: cd: HOME not set\n");
            return 1;
        }
    }

    // Execute change directory
    if (chdir(path) != 0)
    {
        perror("unixsh: cd");
    }
    return 1;
}

int builtin_pwd(command_t *cmd)
{
    (void)cmd; // Silence unused parameter warning
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
    }else{
        perror("unixsh: pwd");
    }
    return 1;
}

int builtin_help(command_t *cmd)
{
    (void)cmd; // Silence unused parameter warning
    printf("Unix Shell (unixsh) - Built-in commands:\n");
    for (int i = 0; builtins[i].name != NULL; i++)
    {
        printf("  %s\n", builtins[i].name);
    }
    return 1;
}

int builtin_exit(command_t *cmd)
{
    (void)cmd; //Silence unused parameter warning
    printf("Goodbye.\n");
    exit(0);
}

int builtin_export(command_t *cmd)
{
    if (cmd->args[1] == NULL)
    {
        fprintf(stderr, "unixsh: export: usage: export NAME=VALUE\n");
        return 1;
    }

    // Look for the '=' character in the argument (e.g., "MYVAR=123")
    char *name = cmd->args[1];
    char *equal_sign = strchr(name, '=');

    if (equal_sign != NULL)
    {
        *equal_sign = '\0'; // Split the string ar '='
        char *value = equal_sign + 1; // The value starts after '='

        if (setenv(name, value, 1) != 0)
        {
            perror("unixsh: export");
        }
    }else{
        fprintf(stderr, "unixsh: export: usage: export NAME=VALUE\n");
    }
    return 1;
}

int builtin_unset(command_t *cmd)
{
    if (cmd->args[1] == NULL)
    {
        fprintf(stderr, "unixsh: unset: usage: unset NAME\n");
        return 1;
    }

    if (unsetenv(cmd->args[1]) != 0)
    {
        perror("unixsh: unset");
    }
    return 1;
}

int builtin_history(command_t *cmd)
{
    (void)cmd; // Silence unused parameter warning
    print_history();
    return 1;
}