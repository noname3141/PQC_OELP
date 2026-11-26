#ifndef NEWHOPE_H
#define NEWHOPE_H

#include "params.h"

#define CRYPTO_SECRETKEYBYTES  NEWHOPE_CCAKEM_SECRETKEYBYTES
#define CRYPTO_PUBLICKEYBYTES  NEWHOPE_CCAKEM_PUBLICKEYBYTES
#define CRYPTO_CIPHERTEXTBYTES NEWHOPE_CCAKEM_CIPHERTEXTBYTES
#define CRYPTO_BYTES           NEWHOPE_SYMBYTES

#if (NEWHOPE_N == 512)
#define CRYPTO_ALGNAME "NewHope512-CCAKEM"
#elif (NEWHOPE_N == 1024)
#define CRYPTO_ALGNAME "NewHope1024-CCAKEM"
#else
#error "NEWHOPE_N must be either 512 or 1024"
#endif

/*
 * Generates public key and private key for CCA-secure NewHope KEM.
 * - pk: Output public key (CRYPTO_PUBLICKEYBYTES bytes)
 * - sk: Output secret key (CRYPTO_SECRETKEYBYTES bytes)
 * Returns 0 on success.
 */
int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);

/*
 * Encapsulates a shared secret ss under recipient's public key pk, generating ciphertext ct.
 * - ct: Output ciphertext (CRYPTO_CIPHERTEXTBYTES bytes)
 * - ss: Output shared secret (CRYPTO_BYTES = 32 bytes)
 * - pk: Recipient public key (CRYPTO_PUBLICKEYBYTES bytes)
 * Returns 0 on success.
 */
int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);

/*
 * Decapsulates ciphertext ct using private key sk to recover shared secret ss.
 * Uses Targhi-Unruh transform with re-encryption check to achieve CCA security.
 * - ss: Output shared secret (CRYPTO_BYTES = 32 bytes)
 * - ct: Input ciphertext (CRYPTO_CIPHERTEXTBYTES bytes)
 * - sk: Input secret key (CRYPTO_SECRETKEYBYTES bytes)
 * Returns 0 on success.
 */
int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);

/* High-level protocol convenience wrappers */
int newhope_keygen(unsigned char *send, unsigned char *sk);
int newhope_sharedb(unsigned char *shared_key, unsigned char *send, const unsigned char *received);
int newhope_shareda(unsigned char *shared_key, const unsigned char *sk, const unsigned char *received);

#endif /* NEWHOPE_H */
