#include "reduce.h"
#include "params.h"

/*
 * Montgomery reduction constants:
 * Modulus q = 12289
 * R = 2^18 = 262144
 * qinv = -q^-1 mod 2^18 = 12287
 */
static const uint32_t qinv = 12287;
static const uint32_t rlog = 18;

/*************************************************
* Name:        montgomery_reduce
* 
* Description: Montgomery reduction: given a 32-bit integer a,
*              computes 16-bit integer congruent to a * R^-1 mod q,
*              where R = 2^18 and q = 12289.
*
* Arguments:   - uint32_t a: input integer to be reduced [0, 1073491968]
*              
* Returns:     integer in [0, 2^14 - 1] congruent to a * R^-1 mod q.
**************************************************/
uint16_t montgomery_reduce(uint32_t a)
{
    /*
     * TODO: Implement Montgomery reduction.
     * 
     * Algorithm steps:
     * 1. Compute u = (a * qinv) mod 2^18.
     *    Hint: Mask with ((1 << rlog) - 1).
     * 2. Compute u = u * NEWHOPE_Q.
     * 3. Compute a = a + u.
     * 4. Return the high bits: (uint16_t)(a >> 18).
     */
    (void)a;
    (void)qinv;
    (void)rlog;
    return 0;
}

/*************************************************
* Name:        barrett_reduce
* 
* Description: Barrett reduction: computes a mod q.
*              Uses the precomputed multiplier 5 / 2^16 to approximate
*              division by q = 12289 (since 5 * 2^16 / 12289 ~= 26.66).
*
* Arguments:   - uint16_t a: input integer [0, 2^14 - 1]
*              
* Returns:     integer in [0, q - 1] congruent to a mod q.
**************************************************/
uint16_t barrett_reduce(uint16_t a)
{
    /*
     * TODO: Implement Barrett reduction mod q = 12289.
     * 
     * Algorithm steps:
     * 1. Approximate quotient u = ((uint32_t)a * 5) >> 16.
     * 2. Multiply by modulus: u = u * NEWHOPE_Q.
     * 3. Subtract from input: a = a - u.
     * 4. Conditional reduction: if (a >= NEWHOPE_Q) a -= NEWHOPE_Q;
     * 5. Return reduced a.
     */
    (void)a;
    return 0;
}
