#ifndef RNG_H
#define RNG_H

#include <stdint.h>

/*
 * xorshift32 による自前の乱数生成器。
 * rand() と違い OS/コンパイラで結果が変わらないため、
 * 同じ seed なら必ず同じマップになる（テスト・不具合再現に使える）。
 * ※ゲーム用。暗号用途には使わないこと。
 */
typedef struct {
    uint32_t state;
} Rng;

void     rng_seed(Rng *r, uint32_t seed);
uint32_t rng_next(Rng *r);
int      rng_range(Rng *r, int n);   /* 0 以上 n 未満（n <= 0 なら 0） */

#endif