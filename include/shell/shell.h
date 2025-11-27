#ifndef SHELL_H
#define SHELL_H

/*
This flag controls the main loop in main.c
It must be accessible by built-in commands (like exit) later,
so we declare it as 'extern' here and define it in main.c
*/
extern int shell_is_running;

#endif