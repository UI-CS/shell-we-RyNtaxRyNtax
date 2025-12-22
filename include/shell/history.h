#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_SIZE 100 // Store last 100 commands

// Function to initialize history system
void init_history(void);

// Function to add a command to history
void add_to_history(const char *cmd);

// Function to get the last command from history
const char *get_last_command(void);

// Function to print all history entries
void print_history(void);

// Function to clean up history memory
void free_history(void);

#endif