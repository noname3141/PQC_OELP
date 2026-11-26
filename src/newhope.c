#include "newhope.h"
#include "cpapke.h"
#include "params.h"
#include "randombytes.h"
#include "fips202.h"
#include "verify.h"
#include <string.h>

/*************************************************
* Name:        crypto_kem_keypair
*
* Description: Generates public and private key for CCA-secure
*              NewHope key encapsulation mechanism (KEM).
*
* sk format: [cpapke_sk (1792B) | cpapke_pk (1824B) | H(pk) (32B) | s (32B)]
* pk format: [cpapke_pk (1824B)]
**************************************************/
int crypto_kem_keypair(unsigned char *pk, unsigned char *sk)
{
    /*
     * TODO: Implement CCA-secure NewHope KEM key generation.
     *
     * Algorithm steps:
     * 1. Generate CPA-PKE keypair: cpapke_keypair(pk, sk).
     * 2. Advance sk pointer past CPA-PKE secret key (1792 bytes).
     * 3. Append CPA-PKE public key pk (1824 bytes) into sk for later re-encryption verification:
     *    for (i = 0; i < NEWHOPE_CPAPKE_PUBLICKEYBYTES; i++) sk[i] = pk[i];
     * 4. Advance sk pointer past pk.
     * 5. Compute and append hash H(pk) = SHAKE-256(pk, 1824, 32 bytes):
     *    shake256(sk, NEWHOPE_SYMBYTES, pk, NEWHOPE_CPAPKE_PUBLICKEYBYTES);
     * 6. Advance sk pointer past H(pk).
     * 7. Append 32 random bytes s (used for pseudo-random return on decapsulation failure):
     *    randombytes(sk, NEWHOPE_SYMBYTES);
     */
    (void)pk;
    (void)sk;
    return 0;
}

/*************************************************
* Name:        crypto_kem_enc
*
* Description: Encapsulates a shared secret ss under recipient public key pk,
*              producing ciphertext ct.
**************************************************/
int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk)
{
    /*
     * TODO: Implement CCA-secure NewHope KEM encapsulation (Targhi-Unruh transform).
     *
     * Algorithm steps:
     * 1. Sample 32 random bytes m: randombytes(buf+1, 32); buf[0] = 0x04.
     * 2. Hash m with domain separation byte: SHAKE-256(buf+1, 32, buf, 33).
     * 3. Hash public key pk: SHAKE-256(buf+1+32, 32, pk, 1824).
     * 4. Derive (k, coins, d) = SHAKE-256(3*32 bytes, [0x08 || hashed_m || H(pk)]):
     *    buf[0] = 0x08; shake256(k_coins_d, 96, buf, 65).
     * 5. Encrypt m using CPA-PKE:
     *    cpapke_enc(ct, buf+1, pk, k_coins_d + NEWHOPE_SYMBYTES).
     * 6. Append Targhi-Unruh confirmation hash d (32 bytes) to ciphertext ct:
     *    memcpy(ct + NEWHOPE_CPAPKE_CIPHERTEXTBYTES, k_coins_d + 2*NEWHOPE_SYMBYTES, 32).
     * 7. Hash ciphertext and derive shared secret:
     *    shake256(k_coins_d + NEWHOPE_SYMBYTES, 32, ct, NEWHOPE_CCAKEM_CIPHERTEXTBYTES);
     *    shake256(ss, 32, k_coins_d, 64).
     */
    (void)ct;
    (void)ss;
    (void)pk;
    return 0;
}

/*************************************************
* Name:        crypto_kem_dec
*
* Description: Decapsulates ciphertext ct using private key sk to recover shared secret ss.
*              Re-encrypts and verifies in constant time against chosen ciphertext attacks (CCA).
**************************************************/
int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk)
{
    /*
     * TODO: Implement CCA-secure NewHope KEM decapsulation.
     *
     * Algorithm steps:
     * 1. Decrypt ciphertext ct with CPA-PKE to recover candidate message m:
     *    buf[0] = 0x08;
     *    cpapke_dec(buf + 1, ct, sk).
     * 2. Retrieve H(pk) stored in sk:
     *    copy H(pk) from sk[NEWHOPE_CCAKEM_SECRETKEYBYTES - 2*NEWHOPE_SYMBYTES] into buf[1 + 32].
     * 3. Recompute (k, coins, d) = SHAKE-256(96, buf, 65).
     * 4. Re-encrypt m using pk:
     *    cpapke_enc(ct_cmp, buf + 1, pk, k_coins_d + NEWHOPE_SYMBYTES).
     * 5. Append d to ct_cmp.
     * 6. Constant-time comparison:
     *    fail = verify(ct, ct_cmp, NEWHOPE_CCAKEM_CIPHERTEXTBYTES).
     * 7. Constant-time conditional move: if fail, replace k with fallback random seed s stored in sk:
     *    shake256(k_coins_d + 32, 32, ct, NEWHOPE_CCAKEM_CIPHERTEXTBYTES);
     *    cmov(k_coins_d, sk + NEWHOPE_CCAKEM_SECRETKEYBYTES - NEWHOPE_SYMBYTES, 32, fail).
     * 8. Hash to derive final shared secret:
     *    shake256(ss, 32, k_coins_d, 64).
     */
    (void)ss;
    (void)ct;
    (void)sk;
    return 0;
}

/*************************************************
* Name:        newhope_keygen
* Description: High-level protocol wrapper for key generation.
**************************************************/
int newhope_keygen(unsigned char *send, unsigned char *sk)
{
    /*
     * TODO: High-level protocol wrapper.
     * Delegates to crypto_kem_keypair(send, sk).
     */
    return crypto_kem_keypair(send, sk);
}

/*************************************************
* Name:        newhope_sharedb
* Description: High-level protocol wrapper for client encapsulation.
**************************************************/
int newhope_sharedb(unsigned char *shared_key, unsigned char *send, const unsigned char *received)
{
    /*
     * TODO: High-level protocol wrapper.
     * Delegates to crypto_kem_enc(send, shared_key, received).
     */
    return crypto_kem_enc(send, shared_key, received);
}

/*************************************************
* Name:        newhope_shareda
* Description: High-level protocol wrapper for server decapsulation.
**************************************************/
int newhope_shareda(unsigned char *shared_key, const unsigned char *sk, const unsigned char *received)
{
    /*
     * TODO: High-level protocol wrapper.
     * Delegates to crypto_kem_dec(shared_key, received, sk).
     */
    return crypto_kem_dec(shared_key, received, sk);
}
