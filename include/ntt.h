#ifndef NTT_H
#define NTT_H

#include <stdint.h>
#include "params.h"

/* Precomputed twiddle factor tables defined in precomp.c */
extern uint16_t omegas_inv_bitrev_montgomery[];
extern uint16_t gammas_bitrev_montgomery[];
extern uint16_t gammas_inv_montgomery[];

/*
 * In-place bit-reversal permutation on polynomial coefficients.
 */
void bitrev_vector(uint16_t *poly);

/*
 * Pointwise multiplication with twiddle factors in Montgomery domain:
 * poly[i] = montgomery_reduce(poly[i] * factors[i])
 */
void mul_coefficients(uint16_t *poly, const uint16_t *factors);

/*
 * In-place forward/backward Number Theoretic Transform (NTT) core butterfly.
 */
void ntt(uint16_t *poly, const uint16_t *omegas);

#endif /* NTT_H */
