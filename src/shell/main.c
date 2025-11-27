// Needed headers 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../../include/shell/shell.h"
#include "../../include/shell/parser.h"

// Definition for the global flag declared in shell.h
int shell_is_running = 1;

// Function prototype from proc_mgmt.c
void setup_process_management();

// Function prototype from executor.c
void execute_command(command_t *cmd);

int main()
{
    // Call the advanced signal handler setup here:
    setup_process_management();

    while (shell_is_running)
    {
        // Read the user input, displaying "unixsh>" as the prompt
        char *line = readline("unixsh>");
        if (line == NULL)
        {
            // EOF (Ctrl+D) received. Exit the loop.
            shell_is_running = 0;
            printf("Goodbye!");
            continue;
        }
        // If the user enters an empty line (just hits Enter), skip processing
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }
        // Add the input line to the history list
        add_history(line);
        // Parse the command 
        command_t *cmd = parse_input(line);

        if (cmd)
        {
            // Check for the 'exit' command (Built-in)
            if (cmd->args[0] && strcmp(cmd->args[0], "exit") == 0)
            {
                shell_is_running = 0;
                printf("Goodbye.\n");
            }else{
                // Execute the command using fork/execvp logic
                execute_command(cmd);
            }
            free_command(cmd); // Clean up the parsed command memory
        }
        // Free the memory allocated by readline() (This remains at the end)
        free(line);
    }
    // Clean up history before exiting the program
    rl_clear_history();
    return 0;
}