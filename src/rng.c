#include "rng.h"

void rng_seed(Rng *r, uint32_t seed) {
    /* xorshift は状態 0 だと永遠に 0 を返すため避ける */
    r->state = seed ? seed : 0x9E3779B9u;
}

uint32_t rng_next(Rng *r) {
    uint32_t x = r->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->state = x;
    return x;
}

int rng_range(Rng *r, int n) {
    if (n <= 0) return 0;
    return (int)(rng_next(r) % (uint32_t)n);
}