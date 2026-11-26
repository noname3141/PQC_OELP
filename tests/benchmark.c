#include <stdio.h>
#include <time.h>
#include "newhope.h"

#define N_ITER 1000

static double get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void)
{
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
    unsigned char ss[CRYPTO_BYTES];
    int i;

    printf("============================================================\n");
    printf("   NewHope-1024 Performance Benchmark (%d operations)      \n", N_ITER);
    printf("============================================================\n");

    /* Keypair generation */
    double t0 = get_time();
    for (i = 0; i < N_ITER; i++) {
        crypto_kem_keypair(pk, sk);
    }
    double t1 = get_time();
    double time_keygen = (t1 - t0) / N_ITER * 1000.0;

    /* Encapsulation */
    t0 = get_time();
    for (i = 0; i < N_ITER; i++) {
        crypto_kem_enc(ct, ss, pk);
    }
    t1 = get_time();
    double time_enc = (t1 - t0) / N_ITER * 1000.0;

    /* Decapsulation */
    t0 = get_time();
    for (i = 0; i < N_ITER; i++) {
        crypto_kem_dec(ss, ct, sk);
    }
    t1 = get_time();
    double time_dec = (t1 - t0) / N_ITER * 1000.0;

    printf("  KeyGen:  %8.3f ms/op (%8.1f ops/sec)\n", time_keygen, 1000.0 / time_keygen);
    printf("  Encaps:  %8.3f ms/op (%8.1f ops/sec)\n", time_enc, 1000.0 / time_enc);
    printf("  Decaps:  %8.3f ms/op (%8.1f ops/sec)\n", time_dec, 1000.0 / time_dec);
    printf("============================================================\n");
    return 0;
}
