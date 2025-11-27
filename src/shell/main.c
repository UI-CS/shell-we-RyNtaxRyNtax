// Needed headers 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../../include/shell/shell.h"

// Definition for the global flag declared in shell.h
int shell_is_running = 1;

int main(int argc, char *argv[])
{
    while (shell_is_running)
    {
        // Read the user input, displaying "unixsh>" as the prompt
        char *line = readline("unixsh>");
        if (line == NULL)
        {
            // EOF (Ctrl+D) received. Exit the loop.
            shell_is_running = 0;
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
        // Check for the 'exit' command
        if (stcmp(line, "exit") == 0)
        {
            shell_is_running = 0;
            printf("Goodbye.")
        }
        // Free the memory allocated by readline()
        free(line);
    }
    // Clean up history before exiting the program
    rl_clear_history();
    return 0;
}