#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "newhope.h"

#define NTESTS 10

int main(void)
{
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
    unsigned char ss_a[CRYPTO_BYTES];
    unsigned char ss_b[CRYPTO_BYTES];
    int i, j;

    printf("============================================================\n");
    printf("   Running NewHope-1024 CCA-KEM Self-Tests (%d iterations)   \n", NTESTS);
    printf("============================================================\n");

    for (i = 0; i < NTESTS; i++) {
        /* Keypair generation */
        if (crypto_kem_keypair(pk, sk) != 0) {
            fprintf(stderr, "[-] Test %d: crypto_kem_keypair failed!\n", i);
            return 1;
        }

        /* Encapsulation */
        if (crypto_kem_enc(ct, ss_b, pk) != 0) {
            fprintf(stderr, "[-] Test %d: crypto_kem_enc failed!\n", i);
            return 1;
        }

        /* Decapsulation */
        if (crypto_kem_dec(ss_a, ct, sk) != 0) {
            fprintf(stderr, "[-] Test %d: crypto_kem_dec failed!\n", i);
            return 1;
        }

        /* Verify shared secrets match */
        if (memcmp(ss_a, ss_b, CRYPTO_BYTES) != 0) {
            fprintf(stderr, "[-] Test %d: Shared secret mismatch between Alice and Bob!\n", i);
            return 1;
        }

        /* Test CCA security: corrupt ciphertext, verify shared secrets diverge */
        ct[0] ^= 0x01;
        crypto_kem_dec(ss_a, ct, sk);
        if (memcmp(ss_a, ss_b, CRYPTO_BYTES) == 0) {
            fprintf(stderr, "[-] Test %d: CCA failure! Corrupted ciphertext decrypted to original secret!\n", i);
            return 1;
        }

        printf("  Iteration %2d: PASS (Shared Secret: ", i + 1);
        for (j = 0; j < 8; j++) printf("%02x", ss_b[j]);
        printf("...)\n");
    }

    printf("\n[SUCCESS] All %d iterations of NewHope-1024 KEM test passed!\n", NTESTS);
    return 0;
}
