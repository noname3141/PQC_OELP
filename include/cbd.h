#ifndef CBD_H
#define CBD_H

#include "poly.h"

/*
 * Centered Binomial Distribution (CBD) sampler with parameter k=8.
 * Given random bytes in buf, samples coefficients in [-k, k] modulo q.
 */
void cbd(poly *r, const unsigned char *buf);

#endif /* CBD_H */
