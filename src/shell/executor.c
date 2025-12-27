#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../../include/shell/parser.h" // For command_t structure
#include "../../include/shell/builtins.h"

void execute_pipeline(command_t *cmd) 
{
    int num_cmds = 0;
    command_t *temp = cmd;
    
    // Count commands to determine the number of pipes needed
    while (temp) 
    {
        num_cmds++;
        temp = temp->next;
    }

    // Allocate 2 file descriptors (read/write) for each pipe connection
    int pipefds[2 * (num_cmds - 1)];

    for (int i = 0; i < num_cmds - 1; i++) 
    {
        if (pipe(pipefds + i * 2) < 0) 
        {
            perror("unixsh: pipe");
            return;
        }
    }

    int j = 0;
    command_t *curr = cmd;
    while (curr) 
    {
        pid_t pid = fork();
        
        if (pid == 0) 
        {
            // --- CHILD PROCESS ---

            // Redirect STDIN to previous pipe read-end (if not first command)
            if (j != 0) 
            {
                dup2(pipefds[(j - 1) * 2], STDIN_FILENO);
            }

            // Redirect STDOUT to current pipe write-end (if not last command)
            if (curr->next) 
            {
                dup2(pipefds[j * 2 + 1], STDOUT_FILENO);
            }

            // Handle file redirections (Overrides pipe connections)
            if (curr->input_file) 
            {
                int fd_in = open(curr->input_file, O_RDONLY);
                if (fd_in >= 0) 
                {
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }
            }
            if (curr->output_file) 
            {
                int fd_out = open(curr->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out >= 0) 
                {
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                } else {
                    perror("unixsh: input file");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe FDs in the child to prevent hanging
            for (int i = 0; i < 2 * (num_cmds - 1); i++) 
            {
                close(pipefds[i]);
            }

            execvp(curr->args[0], curr->args);
            perror("unixsh");
            exit(EXIT_FAILURE);
        }

        curr = curr->next;
        j++;
    }

    // --- PARENT PROCESS ---
    
    // Parent must close all pipe FDs so children receive EOF signals
    for (int i = 0; i < 2 * (num_cmds - 1); i++) 
    {
        close(pipefds[i]);
    }

    // Wait for foreground processes; background jobs are reaped by SIGCHLD handler
    if (!cmd->is_background) 
    {
        for (int i = 0; i < num_cmds; i++) 
        {
            wait(NULL);
        }
    } else {
        fprintf(stderr, "[Pipeline job running in background]\n");
    }
}

void execute_command(command_t *cmd) 
{
    if (!cmd || !cmd->args[0]) return;

    // Run built-ins in the parent ONLY if they are not part of a pipeline
    if (cmd->next == NULL && is_builtin(cmd)) 
    {
        execute_builtin(cmd);
    } else {
        execute_pipeline(cmd);
    }
}