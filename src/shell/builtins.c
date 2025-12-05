#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../include/shell/builtins.h"

// Array of built-in commands
// The {NULL, NULL} at the end is the "Sentinel" that stops the loops.
builtin_t builtins[] = 
{
    {"cd", builtin_cd},
    {"pwd", builtin_pwd},
    {"help", builtin_help},
    {"exit", builtin_exit},
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