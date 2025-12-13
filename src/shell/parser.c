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

    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
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
    // 1. Check for Output Redirection
    if (strcmp(token, ">") == 0) {
        token = strtok(NULL, " \t\n");
        if (token) cmd->output_file = strdup(token);
    } 
    // 2. Check for Input Redirection
    else if (strcmp(token, "<") == 0) {
        token = strtok(NULL, " \t\n");
        if (token) cmd->input_file = strdup(token);
    }
    // 3. Handle Background Operator '&'
    else 
    {
        size_t len = strlen(token);
        // If the token IS exactly "&" or ENDS with "&" (like "sleep 5&")
        if (token[len - 1] == '&') {
            cmd->is_background = 1;
            
            if (len > 1) {
                // Remove '&' from the string (e.g., "5&" becomes "5")
                token[len - 1] = '\0';
                cmd->args[i++] = strdup(token);
            }
            // If it was just "&", we don't add it to args, just set the flag.
        } else {
            // Normal argument
            cmd->args[i++] = strdup(token);
        }
    }
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