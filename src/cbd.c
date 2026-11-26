#include "cbd.h"
#include "params.h"

/*************************************************
* Name:        hw
* 
* Description: Compute the Hamming weight of a byte
**************************************************/
static unsigned char hw(unsigned char a)
{
    unsigned char i, r = 0;
    for (i = 0; i < 8; i++)
        r += (a >> i) & 1;
    return r;
}

/*************************************************
* Name:        cbd
* 
* Description: Given an array of uniformly random bytes, computes
*              polynomial with coefficients according to the centered
*              binomial distribution with parameter k=8.
*
* Arguments:   - poly *r:                output polynomial
*              - const unsigned char *buf: input random bytes (128 bytes per 64 coeffs)
**************************************************/
void cbd(poly *r, const unsigned char *buf)
{
    /*
     * TODO: Implement Centered Binomial Distribution (CBD) sampling for k=8.
     * 
     * Mathematical background:
     * A centered binomial variable with parameter k=8 is the difference of two
     * binomial variables B(k, 1/2), each obtained by summing 8 independent random bits:
     *    X = sum_{i=1}^k b_i - sum_{i=1}^k b'_i  in [-8, 8].
     * In Z_q, negative values are represented by adding q:
     *    coeff = hw(a) + NEWHOPE_Q - hw(b).
     *
     * Algorithm steps:
     * 1. For each coefficient j from 0 to 63:
     *    - Read two consecutive bytes: a = buf[2*j], b = buf[2*j + 1].
     *    - Compute Hamming weights: hw(a) and hw(b).
     *    - Set r->coeffs[j] = hw(a) + NEWHOPE_Q - hw(b).
     */
    (void)r;
    (void)buf;
    (void)hw;
}
