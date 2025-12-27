#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/mman.h>
#include <errno.h>
#include "../../include/utils/utils.h"

void *create_shared_memory(size_t size)
{
    if (size == 0) {
        fprintf(stderr, "Error: Cannot allocate shared memory of size 0\n");
        errno = EINVAL;
        return NULL;
    }
    
    // Create an anonymous shared memory mapping 
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap failed");
        return NULL;
    }
    
    // Initialize memory to zero
    memset(ptr, 0, size);
    return ptr;
}

int detach_shared_memory(void *ptr, size_t size)
{
    if (ptr == NULL) {
        fprintf(stderr, "Error: Cannot detach NULL pointer\n");
        return -1;
    }
    
    if (size == 0) {
        fprintf(stderr, "Error: Invalid size (0) for shared memory\n");
        return -1;
    }
    
    int result = munmap(ptr, size);
    if (result == -1) {
        perror("munmap failed");
    }
    return result;
}