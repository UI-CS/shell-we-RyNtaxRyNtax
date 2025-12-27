#include <stdatomic.h>
#include "../../include/utils/atomic.h"

// A shared counter that apps can use 
void increment_atomic_counter(atomic_int *counter)
{
    atomic_fetch_add(counter, 1);
}

void set_atomic_val(atomic_int *counter, int val)
{
    atomic_store(counter, val);
}