#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "poly.h"
#include "ntt.h"
#include "reduce.h"
#include "randombytes.h"

int main(void)
{
    poly original, transformed;
    int i, trial;

    printf("============================================================\n");
    printf("   Running Number Theoretic Transform (NTT) Roundtrip Tests  \n");
    printf("============================================================\n");

    for (trial = 0; trial < 10; trial++) {
        /* Generate random polynomial coefficients in [0, Q-1] */
        for (i = 0; i < NEWHOPE_N; i++) {
            uint16_t val;
            randombytes((unsigned char *)&val, 2);
            original.coeffs[i] = val % NEWHOPE_Q;
            transformed.coeffs[i] = original.coeffs[i];
        }

        /* 
         * NewHope forward NTT expects input coefficients in bit-reversed order,
         * and outputs coefficients in standard order.
         * Inverse NTT expects standard order, and outputs standard order.
         */
        bitrev_vector(transformed.coeffs);
        poly_ntt(&transformed);
        poly_invntt(&transformed);

        /* Verify every coefficient matches original modulo Q */
        for (i = 0; i < NEWHOPE_N; i++) {
            uint16_t r = barrett_reduce(transformed.coeffs[i]);
            if (r != original.coeffs[i]) {
                fprintf(stderr, "[-] INTT(NTT(p)) mismatch at coeff %d: expected %u, got %u\n",
                        i, original.coeffs[i], r);
                return 1;
            }
        }
        printf("  NTT roundtrip trial %2d: PASS\n", trial + 1);
    }

    printf("\n[SUCCESS] NTT forward and inverse transforms verified bit-exact!\n");
    return 0;
}
