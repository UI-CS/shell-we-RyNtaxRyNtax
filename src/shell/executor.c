#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../include/shell/parser.h" // For command_t structure

// Function prototypes
void execute_command(command_t *cmd);
int is_builtit_command(command_t *cmd);
void execute_builtin_command(command_t *cmd);
void execute_external_command(command_t *cmd);

/*
Executes a command based on its type (built-in or external)
*/
void execute_command(command_t *cmd)
{
    if (!cmd || !cmd->args[0])
    {
        return; // Empty command
    }

    // External command requires fork/exec
    execute_external_command(cmd);
}

// Built-in check
int is_builtin_command(command_t *cmd)
{
    // I already handled 'exit' in main.c, but i should add others here later.
    // For now, i only need 'exit' and this function will return 0 for everything else.
    return 0;
}

// Built-in execution
void execute_builtin_command(command_t *cmd)
{
    // Nothing here yet, as 'exit' is handled in main.c
    (void)cmd;
}

/*
Handles fork, execvp, and waiting for external commands
*/
void execute_external_command(command_t *cmd)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("unixsh: fork failed");
        return;
    }

    if (pid == 0)
    {
        // --- CHILD PROCESS ---
        // Execute the command using the NULL-terminated args array
        execvp(cmd->args[0], cmd->args);

        // execvp only returns if an error occurred
        perror("unixsh: command execution failed");
        // Child must exit immediately upon failure
        exit(EXIT_FAILURE);
    }else{
        // --- PARENT PROCESS --
        if (cmd->is_background)
        {
            // Background command: Parent prints PID and returns immediately
            fprintf(stderr, "[job %d running in background]\n", pid);

        }else{
            // Foreground command: Parent blocks and waits for child to finish
            int status;
            do
            {
                // Use waitpid to specifically wait for the foreground child
                // Loop is important to handle signal interruptions
            }while (waitpid(pid, &status, 0) == -1);
        }
    }
}