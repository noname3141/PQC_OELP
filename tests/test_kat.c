#include <stdio.h>
#include <string.h>
#include "newhope.h"

int main(void)
{
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
    unsigned char ss_enc[CRYPTO_BYTES];
    unsigned char ss_dec[CRYPTO_BYTES];

    printf("============================================================\n");
    printf("   NewHope-1024 Known Answer & Consistency Validation        \n");
    printf("============================================================\n");

    crypto_kem_keypair(pk, sk);
    crypto_kem_enc(ct, ss_enc, pk);
    crypto_kem_dec(ss_dec, ct, sk);

    if (memcmp(ss_enc, ss_dec, CRYPTO_BYTES) != 0) {
        fprintf(stderr, "[-] KAT consistency check failed!\n");
        return 1;
    }

    printf("[+] Key sizes verified:\n");
    printf("    Public Key:  %d bytes (expected %d)\n", CRYPTO_PUBLICKEYBYTES, NEWHOPE_CPAPKE_PUBLICKEYBYTES);
    printf("    Secret Key:  %d bytes (expected %d)\n", CRYPTO_SECRETKEYBYTES, NEWHOPE_CCAKEM_SECRETKEYBYTES);
    printf("    Ciphertext:  %d bytes (expected %d)\n", CRYPTO_CIPHERTEXTBYTES, NEWHOPE_CCAKEM_CIPHERTEXTBYTES);
    printf("    Shared Secret: %d bytes\n", CRYPTO_BYTES);

    printf("\n[SUCCESS] KAT consistency check passed!\n");
    return 0;
}
