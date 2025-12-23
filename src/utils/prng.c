#include <stdint.h>
#include <string.h>
#include "../../include/utils/prng.h"

// ==================== PCG32 Implementation ====================
void pcg32_init(pcg32_state_t *rng, uint64_t seed) 
{
    rng->state = 0U;
    rng->increment = (seed << 1u) | 1u;
    pcg32_next(rng);
    rng->state += seed;
    pcg32_next(rng);
}

uint64_t pcg32_next(pcg32_state_t *rng) 
{
    uint64_t oldstate = rng->state;
    
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + rng->increment;
    
    // Calculate output function (XSH RR)
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// ==================== Xorshift128+ Implementation ====================
void xorshift128_init(xorshift128_state_t *rng, uint64_t seed) 
{
    // SplitMix64 to initialize the state
    splitmix64_state_t sm;
    splitmix64_init(&sm, seed);
    
    rng->x = splitmix64_next(&sm);
    rng->y = splitmix64_next(&sm);
    
    // Ensure state is non-zero
    if (rng->x == 0 && rng->y == 0) 
    {
        rng->x = 0x123456789ABCDEFULL;
    }
}

uint64_t xorshift128_next(xorshift128_state_t *rng) 
{
    uint64_t x = rng->x;
    uint64_t y = rng->y;
    
    rng->x = y;
    x ^= x << 23;
    rng->y = x ^ y ^ (x >> 17) ^ (y >> 26);
    
    return rng->y + y;
}

// ==================== SplitMix64 Implementation ====================
void splitmix64_init(splitmix64_state_t *rng, uint64_t seed) 
{
    rng->seed = seed;
}

uint64_t splitmix64_next(splitmix64_state_t *rng) 
{
    uint64_t z = (rng->seed += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

// ==================== Generic PRNG Interface ====================
void prng_init(prng_t *prng, uint64_t seed, prng_type_t type) 
{
    prng->type = type;
    
    switch (type) 
    {
        case PCG32:
            pcg32_init(&prng->state.pcg32, seed);
            break;
        case XORSHIFT128:
            xorshift128_init(&prng->state.xorshift128, seed);
            break;
        case SPLITMIX64:
            splitmix64_init(&prng->state.splitmix64, seed);
            break;
        default:
            // Default to PCG32
            pcg32_init(&prng->state.pcg32, seed);
            break;
    }
}

uint64_t prng_next(prng_t *prng) 
{
    switch (prng->type) 
    {
        case PCG32:
            return pcg32_next(&prng->state.pcg32);
        case XORSHIFT128:
            return xorshift128_next(&prng->state.xorshift128);
        case SPLITMIX64:
            return splitmix64_next(&prng->state.splitmix64);
        default:
            return pcg32_next(&prng->state.pcg32);
    }
}

double prng_next_double(prng_t *prng) 
{
    // Generate random double in [0,1) with correct scaling
    uint64_t r = prng_next(prng);
    return uint64_to_double(r, prng->type);
}