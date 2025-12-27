#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

// Shared memory helpers
void *create_shared_memory(size_t size);
int detach_shared_memory(void *ptr, size_t size);

// Common macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif