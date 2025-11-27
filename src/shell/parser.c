// To expose POSIX features like strdup
#define _XOPEN_SOURCE 700

// Needed headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/shell/parser.h"

// Frees all memory associated with the command_t structure
void free_command(command_t *cmd)
{
    if (!cmd) return;

    // Loop through the arguments array until the NULL terminator
    for (int i  = 0; cmd->args[i] != NULL; i++)
    {
        free(cmd->args[i]);
    }
    free(cmd);
}

// Tokenize the input line into command_t structure
command_t *parse_input(const char *line)
{
    if (!line || strlen(line) == 0) return NULL;

    command_t *cmd = (command_t *)malloc(sizeof(command_t));
    if (!cmd) return NULL;

    memset(cmd, 0, sizeof(command_t));
    cmd->is_background = 0;

    // Create a mutable copy of the input line for strtok
    char *line_copy = strdup(line);
    char *token;
    int i = 0;

    // Tokenize using space, tab, and newline as delimiters
    token = strtok(line_copy, " \t\n");
    
    while (token != NULL && i < MAX_ARGS - 1)
    {
        // Check for the background operator '&' at the end of the input
        if (strcmp(token, "&") == 0)
        {
            // Check if '&' is the very last token
            if (strtok(NULL, " \t\n") == NULL)
            {
                cmd->is_background = 1;
                break; // Stop parsing, don't store '&'
            }
        }
        cmd->args[i] = strdup(token);
        i++;
        token = strtok(NULL, " \t\n");
    }

    cmd->args[i] = NULL; // NULL-terminate the argument list
    free(line_copy);

    // Check if any arguments were parsed successfully
    if (i == 0 && cmd->is_background == 0)
    {
        free_command(cmd); // Nothing was parsed
        return NULL;
    }
    return cmd;
}