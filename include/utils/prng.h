#ifndef PRNG_H
#define PRNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

// PRNG types
typedef enum
{
    PCG32,
    XORSHIFT128,
    SPLITMIX64
} prng_type_t;

// PRNG state structures
typedef struct
{
    uint64_t state;
    uint64_t increment;
} pcg32_state_t;

typedef struct 
{
    uint64_t x, y;
} xorshift128_state_t;

typedef struct 
{
    uint64_t seed;
} splitmix64_state_t;

// Union for different PRNG states
typedef union 
{
    pcg32_state_t pcg32;
    xorshift128_state_t xorshift128;
    splitmix64_state_t splitmix64;
} prng_state_u;

// Main PRNG structure
typedef struct 
{
    prng_type_t type;
    prng_state_u state;
} prng_t;

// Thread worker arguments
typedef struct 
{
    uint64_t seed;
    prng_type_t prng_type;
    uint64_t iterations;
    _Atomic uint_fast64_t *total_inside;
    _Atomic uint_fast64_t *total_points;
} thread_worker_args_t;

// PRNG functions
void prng_init(prng_t *prng, uint64_t seed, prng_type_t type);
uint64_t prng_next(prng_t *prng);
double prng_next_double(prng_t *prng);

// Individual PRNG implementations
uint64_t pcg32_next(pcg32_state_t *rng);
void pcg32_init(pcg32_state_t *rng, uint64_t seed);

uint64_t xorshift128_next(xorshift128_state_t *rng);
void xorshift128_init(xorshift128_state_t *rng, uint64_t seed);

uint64_t splitmix64_next(splitmix64_state_t *rng);
void splitmix64_init(splitmix64_state_t *rng, uint64_t seed);

// Utility functions
static inline uint64_t rotl64(uint64_t x, int k) 
{
    return (x << k) | (x >> (64 - k));
}

static inline double uint64_to_double(uint64_t x, prng_type_t type) 
{
    // Different scaling for 32-bit vs 64-bit PRNGs
    switch (type) 
    {
        case PCG32:
            // PCG32 generates 32-bit values, scale by 2^32
            return (double)x / 4294967296.0;  // 2^32
            
        case XORSHIFT128:
        case SPLITMIX64:
            // These generate 64-bit values
            return (double)x / 18446744073709551616.0;  // 2^64
            
        default:
            return (double)x / 18446744073709551616.0;
    }
}

#endif