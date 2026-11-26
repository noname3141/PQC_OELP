#include <stdio.h>
#include <stdint.h>
#include "poly.h"
#include "cbd.h"
#include "randombytes.h"

int main(void)
{
    poly p;
    unsigned char seed[NEWHOPE_SYMBYTES];
    int i;
    int64_t sum = 0;

    printf("============================================================\n");
    printf("   Running Centered Binomial Distribution (CBD) Tests        \n");
    printf("============================================================\n");

    randombytes(seed, NEWHOPE_SYMBYTES);
    poly_sample(&p, seed, 0);

    /* For k=8, noise coefficients centered around 0 in [-8, 8] represented mod Q */
    int min_val = 99999, max_val = -99999;
    for (i = 0; i < NEWHOPE_N; i++) {
        int val = (int)p.coeffs[i];
        if (val > NEWHOPE_Q / 2) val -= NEWHOPE_Q; /* centered signed value */

        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
        sum += val;

        if (val < -8 || val > 8) {
            fprintf(stderr, "[-] CBD coefficient out of [-8, 8] bounds: %d (raw: %u)\n", val, p.coeffs[i]);
            return 1;
        }
    }

    double mean = (double)sum / NEWHOPE_N;
    printf("[+] CBD sampled %d coefficients from seed\n", NEWHOPE_N);
    printf("[+] Bounds: min = %d, max = %d (expected within [-8, 8])\n", min_val, max_val);
    printf("[+] Sample mean: %.4f (expected close to 0.0)\n", mean);

    printf("\n[SUCCESS] CBD noise distribution verified within bounds!\n");
    return 0;
}
