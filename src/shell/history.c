// To expose POSIX features like strdup
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/shell/history.h"

static char *history[HISTORY_SIZE];
static int history_count = 0;
static int history_index = 0; // Next position to write 
static int history_start = 0; // Oldest entry index (for circular buffer)

void init_history(void)
{
    // Initialize all pointers to `NULL`
    memset(history, 0, sizeof(history));
    history_count = 0;
    history_index = 0;
    history_start = 0;
}

void add_to_history(const char *cmd)
{
    // Don't store empty commands
    if (!cmd || strlen(cmd) == 0) return; 

    // Free old entry if exists
    if (history[history_index]) free(history[history_index]);

    // Store new command
    history[history_index] = strdup(cmd);
    if (!history[history_index])
    {
        perror("history: strduop");
        return;
    }

    // Move index (circular buffer)
    history_index = (history_index + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE)
    {
        history_count++;
    }else{
        history_start = (history_start + 1) % HISTORY_SIZE;
    }
}

const char *get_last_command(void)
{
    if (history_count == 0) return NULL;

    // Calculate index of last command
    int last_index = (history_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    return history[last_index];
}

void print_history(void)
{
    if (history_count == 0)
    {
        printf("No commands in history\n");
        return;
    }

    printf("Command History (most recent last):\n");
    printf("===================================\n");
    
    // Print from oldestto newest
    for (int  i = 0; i < history_count; i++)
    {
        int idx = (history_start + i) % HISTORY_SIZE;
        printf("%3d: %s\n", i + 1, history[idx]);
    }
}

void free_history(void)
{
    for (int i = 0; i < HISTORY_SIZE; i++)
    {
        if (history[i])
        {
            free(history[i]);
            history[i] = NULL;
        }
    }
    history_count = 0;
    history_index = 0;
    history_start = 0;
}