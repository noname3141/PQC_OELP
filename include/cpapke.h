#ifndef CPAPKE_H
#define CPAPKE_H

#include "params.h"

/*
 * CPA-secure Public Key Encryption Key Generation:
 * Generates public key pk and private key sk.
 * - pk: NEWHOPE_CPAPKE_PUBLICKEYBYTES (1824 bytes) = poly a_hat (1792 bytes) + seed (32 bytes)
 * - sk: NEWHOPE_CPAPKE_SECRETKEYBYTES (1792 bytes) = poly s_hat
 */
void cpapke_keypair(unsigned char *pk, unsigned char *sk);

/*
 * CPA-secure Public Key Encryption:
 * Encrypts 32-byte message into ciphertext ct using public key pk and coins.
 * - ct: NEWHOPE_CPAPKE_CIPHERTEXTBYTES (2176 bytes) = u (1792 bytes) + h (384 bytes)
 * - msg: 32 bytes
 * - pk: NEWHOPE_CPAPKE_PUBLICKEYBYTES (1824 bytes)
 * - coins: 32 bytes (randomness seed)
 */
void cpapke_enc(unsigned char *ct, const unsigned char *msg, const unsigned char *pk, const unsigned char *coins);

/*
 * CPA-secure Public Key Decryption:
 * Decrypts ciphertext ct using secret key sk to recover 32-byte message msg.
 * - msg: 32 bytes output
 * - ct: NEWHOPE_CPAPKE_CIPHERTEXTBYTES (2176 bytes)
 * - sk: NEWHOPE_CPAPKE_SECRETKEYBYTES (1792 bytes)
 */
void cpapke_dec(unsigned char *msg, const unsigned char *ct, const unsigned char *sk);

#endif /* CPAPKE_H */
