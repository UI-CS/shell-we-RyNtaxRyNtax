#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64

// Structure to hold the parsed command data
typedef struct
{
    char *args[MAX_ARGS]; // NULL-terminated array for execvp()
    int is_background;   // Flag for '&' execution (1 if true, 0 if false)
    char *input_file; // For '<'
    char *output_file; // For '>'
} command_t;

// Function to parse the input line 
command_t *parse_input(const char *line);

// Function to free the memory used by command_t
void free_command(command_t *cmd);

#endif