#include "random.h"

static uint32_t rng_state = 1;

uint16_t rand_simple(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

