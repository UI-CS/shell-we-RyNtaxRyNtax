#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdatomic.h>
#include <stdbool.h>

// Wrapper functions to avoid name conflicts with standard library
static inline void atomic_flag_init(atomic_flag *flag) 
{
    atomic_flag_clear(flag);
}

static inline bool atomic_flag_try_lock(atomic_flag *flag) 
{
    return !atomic_flag_test_and_set(flag);
}

static inline void atomic_flag_lock(atomic_flag *flag) 
{
    while (atomic_flag_test_and_set(flag)) 
    {
        // Spin wait
    }
}

static inline void atomic_flag_unlock(atomic_flag *flag) 
{
    atomic_flag_clear(flag);
}

// Atomic increment
static inline int atomic_inc(atomic_int *value) 
{
    return atomic_fetch_add(value, 1) + 1;
}

// Atomic add
static inline int atomic_add_val(atomic_int *value, int amount) 
{
    return atomic_fetch_add(value, amount) + amount;
}

// Atomic store
static inline void atomic_set(atomic_int *value, int new_value) 
{
    atomic_store(value, new_value);
}

// Atomic load
static inline int atomic_get(atomic_int *value) 
{
    return atomic_load(value);
}

// Compare and swap
static inline bool atomic_cas(atomic_int *value, int expected, int new_val) 
{
    return atomic_compare_exchange_strong(value, &expected, new_val);
}

void increment_atomic_counter(atomic_int *counter);
void set_atomic_val(atomic_int *counter, int val);

#endif