#ifndef REDUCE_H
#define REDUCE_H

#include <stdint.h>
#include "params.h"

/*
 * Montgomery reduction:
 * Given a 32-bit integer a, computes 16-bit integer congruent to a * R^-1 mod q,
 * where R = 2^18 and q = 12289.
 * Valid input range: a in [0, 1073491968].
 * Output range: [0, 2^14 - 1].
 */
uint16_t montgomery_reduce(uint32_t a);

/*
 * Barrett reduction:
 * Computes 16-bit integer congruent to a mod q, where q = 12289.
 * Output range: [0, q - 1].
 */
uint16_t barrett_reduce(uint16_t a);

#endif /* REDUCE_H */
