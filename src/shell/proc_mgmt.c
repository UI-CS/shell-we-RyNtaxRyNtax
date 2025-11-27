// To ensure POSIX features (like sigaction and SA_RESTART) are exposed.
#define _XOPEN_SOURCE 700 

// Needed headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

// Functions declaration
void handle_sigchld(int sig)
{
    // This line silences the "unused parameter 'sig'" warning cleanly
    (void)sig; 

    int status;
    pid_t pid;

    // Use a non-blocking loop to reap all terminated children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // Note: Using fprintf to stderr prevents the message from being piped or redirected.
        fprintf(stderr, "\n[Job %d finished]\n", pid);
    }
}

void setup_process_management()
{
    struct sigaction sa;
    
    // Initialize the structure fields to zero
    // Using memset is crucial since the struct sigaction has complex internal members
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask); // Do not block any signals while handling SIGCHLD

    // Set advanced flags
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    // Register the handler for the SIGCHLD signal
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
        perror("Fatal: sigaction failed to set SIGCHLD handler");
        exit(EXIT_FAILURE);
    }
}