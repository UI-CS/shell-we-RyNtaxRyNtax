// To expose POSIX features like strdup
#define _XOPEN_SOURCE 700

// Needed headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/shell/parser.h"
#include "../../include/shell/history.h"

// Frees all memory associated with the command_t structure
void free_command(command_t *cmd)
{
    if (!cmd) return;

    // 1. Recursive call: Free the next command in the pipeline first
    if (cmd->next)
    {
        free_command(cmd->next);
    }

    // 2. Free arguments of current command
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

    char *expanded_line = NULL;

    // Handle `!!` expansion
    if (strcmp(line, "!!") == 0)
    {
        const char *last_cmd = get_last_command();
        if (!last_cmd)
        {
            fprintf(stderr, "unixsh: !!: No previous command\n");
            return NULL;
        }
    expanded_line = strdup(last_cmd);
    fprintf(stderr, "!! -> %s\n", expanded_line); // Echo expansion
    }else{
        expanded_line = strdup(line);
    }

    if (!expanded_line) return NULL;

    // Initial command setup
    command_t *head = (command_t *)malloc(sizeof(command_t));
    if (!head)
    {
        free(expanded_line);
        return NULL;
    }

    memset(head, 0, sizeof(command_t));
    command_t *current_cmd = head;

    // Create a mutable copy of the input line for strtok
    char *line_copy = strdup(expanded_line);
    free(expanded_line);
    
    // Tokenize using space, tab, and newline as delimiters
    char *token = strtok(line_copy, " \t\n");
    int i = 0;

    while (token != NULL)
    {
        // --- PIPE DETECTION ---
        if (strcmp(token, "|") == 0) 
        {
            current_cmd->args[i] = NULL; // Terminate current args
            
            // Create next command in the chain
            command_t *next_cmd = (command_t *)malloc(sizeof(command_t));
            memset(next_cmd, 0, sizeof(command_t));
            
            current_cmd->next = next_cmd;
            current_cmd = next_cmd; // Move pointer to the new command
            i = 0;                  // Reset arg counter for new command
        } 
        // --- REDIRECTION & BACKGROUND LOGIC ---
        else if (strcmp(token, ">") == 0) 
        {
            token = strtok(NULL, " \t\n");
            if (token) current_cmd->output_file = strdup(token);
        } 
        else if (strcmp(token, "<") == 0) 
        {
            token = strtok(NULL, " \t\n");
            if (token) current_cmd->input_file = strdup(token);
        }
        else 
        {
            size_t len = strlen(token);
            // If the token IS exactly "&" or ENDS with "&" (like "sleep 5&")
            if (token[len - 1] == '&') 
            {
                // Background flag applies to the WHOLE pipeline
                head->is_background = 1; 
                if (len > 1) 
                {
                    // Remove '&' from the string (e.g., "5&" becomes "5")
                    token[len - 1] = '\0';
                    current_cmd->args[i++] = strdup(token);
                }
            } else {
                // If it was just "&", we don't add it to args, just set the flag.
                if (i < MAX_ARGS - 1) 
                {
                    current_cmd->args[i++] = strdup(token);
                }
            }
        }
        token = strtok(NULL, " \t\n");
    }

    current_cmd->args[i] = NULL; 
    free(line_copy);

    // Basic validation: ensure the first command has at least one argument
    if (head->args[0] == NULL) {
        free_command(head);
        return NULL;
    }
    return head;
}