#include "cpapke.h"
#include "poly.h"
#include "randombytes.h"
#include "fips202.h"

/*************************************************
* Name:        encode_pk
* 
* Description: Serialize public key as polynomial b || seed.
**************************************************/
static void encode_pk(unsigned char *r, const poly *pk, const unsigned char *seed)
{
    /*
     * TODO: Implement public key serialization.
     * 1. Serialize polynomial pk to bytes (NEWHOPE_POLYBYTES = 1792 bytes): poly_tobytes(r, pk).
     * 2. Append public 32-byte seed directly after: r[NEWHOPE_POLYBYTES + i] = seed[i].
     */
    (void)r;
    (void)pk;
    (void)seed;
}

/*************************************************
* Name:        decode_pk
* 
* Description: Deserialize public key back to polynomial b and seed.
**************************************************/
static void decode_pk(poly *pk, unsigned char *seed, const unsigned char *r)
{
    /*
     * TODO: Implement public key deserialization.
     * 1. Deserialize polynomial pk from the first NEWHOPE_POLYBYTES bytes: poly_frombytes(pk, r).
     * 2. Copy the trailing 32 bytes into seed: seed[i] = r[NEWHOPE_POLYBYTES + i].
     */
    (void)pk;
    (void)seed;
    (void)r;
}

/*************************************************
* Name:        encode_c
* 
* Description: Serialize ciphertext: polynomial u (1792 bytes) || compressed v (384 bytes).
**************************************************/
static void encode_c(unsigned char *r, const poly *b, const poly *v)
{
    /*
     * TODO: Implement ciphertext serialization.
     * 1. Serialize polynomial b (u) to bytes (NEWHOPE_POLYBYTES = 1792 bytes): poly_tobytes(r, b).
     * 2. Compress and serialize polynomial v (384 bytes): poly_compress(r + NEWHOPE_POLYBYTES, v).
     */
    (void)r;
    (void)b;
    (void)v;
}

/*************************************************
* Name:        decode_c
* 
* Description: Deserialize ciphertext back to polynomial u and compressed v.
**************************************************/
static void decode_c(poly *b, poly *v, const unsigned char *r)
{
    /*
     * TODO: Implement ciphertext deserialization.
     * 1. Deserialize polynomial b (u) from first 1792 bytes: poly_frombytes(b, r).
     * 2. Decompress polynomial v from trailing 384 bytes: poly_decompress(v, r + NEWHOPE_POLYBYTES).
     */
    (void)b;
    (void)v;
    (void)r;
}

/*************************************************
* Name:        gen_a
* 
* Description: Deterministically expand public polynomial a from public seed.
**************************************************/
static void gen_a(poly *a, const unsigned char *seed)
{
    /*
     * TODO: Expand public polynomial a from seed.
     * Call poly_uniform(a, seed).
     */
    (void)a;
    (void)seed;
}

/*************************************************
* Name:        cpapke_keypair
* 
* Description: Generates public key pk and private key sk for CPA-PKE.
*              pk = (b = a*s + e, publicseed)
*              sk = s (in NTT domain)
**************************************************/
void cpapke_keypair(unsigned char *pk, unsigned char *sk)
{
    /*
     * TODO: Implement CPA-secure public key encryption key generation.
     * 
     * Algorithm steps:
     * 1. Generate 64 bytes of randomness using SHAKE-256(0x01 || randombytes(32)):
     *    - z[0] = 0x01; randombytes(z+1, 32); shake256(z, 64, z, 33).
     *    - First 32 bytes = publicseed (for public matrix/polynomial a).
     *    - Second 32 bytes = noiseseed (for secret s and error e).
     * 2. Expand public polynomial a: gen_a(&ahat, publicseed).
     * 3. Sample secret s ~ CBD(noiseseed, nonce=0) and convert to NTT:
     *    poly_sample(&shat, noiseseed, 0); poly_ntt(&shat).
     * 4. Sample error e ~ CBD(noiseseed, nonce=1) and convert to NTT:
     *    poly_sample(&ehat, noiseseed, 1); poly_ntt(&ehat).
     * 5. Compute public polynomial:
     *    b = a * s + e (in NTT domain):
     *    poly_mul_pointwise(&ahat_shat, &shat, &ahat);
     *    poly_add(&bhat, &ehat, &ahat_shat);
     * 6. Serialize sk = s_hat (1792 bytes): poly_tobytes(sk, &shat).
     * 7. Serialize pk = b_hat || publicseed (1824 bytes): encode_pk(pk, &bhat, publicseed).
     */
    (void)pk;
    (void)sk;
    (void)encode_pk;
    (void)decode_pk;
    (void)encode_c;
    (void)decode_c;
    (void)gen_a;
}

/*************************************************
* Name:        cpapke_enc
* 
* Description: CPA-secure Public Key Encryption.
*              Encrypts 32-byte message into ciphertext ct.
**************************************************/
void cpapke_enc(unsigned char *ct,
                const unsigned char *msg,
                const unsigned char *pk,
                const unsigned char *coins)
{
    /*
     * TODO: Implement CPA-secure public key encryption.
     *
     * Algorithm steps:
     * 1. Encode 32-byte message msg into polynomial v (coeffs = 0 or Q/2):
     *    poly_frommsg(&v, msg).
     * 2. Deserialize public key pk into (bhat, publicseed):
     *    decode_pk(&bhat, publicseed, pk).
     * 3. Re-generate public polynomial ahat:
     *    gen_a(&ahat, publicseed).
     * 4. Sample secret s' ~ CBD(coins, nonce=0) and convert to NTT:
     *    poly_sample(&sprime, coins, 0); poly_ntt(&sprime).
     * 5. Sample error e' ~ CBD(coins, nonce=1) and convert to NTT:
     *    poly_sample(&eprime, coins, 1); poly_ntt(&eprime).
     * 6. Sample error e'' ~ CBD(coins, nonce=2):
     *    poly_sample(&eprimeprime, coins, 2).
     * 7. Compute uhat = ahat * sprime + eprime (in NTT domain):
     *    poly_mul_pointwise(&uhat, &ahat, &sprime);
     *    poly_add(&uhat, &uhat, &eprime);
     * 8. Compute vprime = INTT(bhat * sprime) + e'' + v:
     *    poly_mul_pointwise(&vprime, &bhat, &sprime);
     *    poly_invntt(&vprime);
     *    poly_add(&vprime, &vprime, &eprimeprime);
     *    poly_add(&vprime, &vprime, &v);
     * 9. Serialize ciphertext ct = (uhat, compress(vprime)):
     *    encode_c(ct, &uhat, &vprime);
     */
    (void)ct;
    (void)msg;
    (void)pk;
    (void)coins;
}

/*************************************************
* Name:        cpapke_dec
* 
* Description: CPA-secure Public Key Decryption.
*              Decrypts ciphertext ct to recover 32-byte message.
**************************************************/
void cpapke_dec(unsigned char *msg,
                const unsigned char *ct,
                const unsigned char *sk)
{
    /*
     * TODO: Implement CPA-secure public key decryption.
     *
     * Algorithm steps:
     * 1. Deserialize secret key sk into polynomial shat (in NTT domain):
     *    poly_frombytes(&shat, sk).
     * 2. Deserialize ciphertext ct into uhat and decompressed vprime:
     *    decode_c(&uhat, &vprime, ct).
     * 3. Compute INTT(shat * uhat):
     *    poly_mul_pointwise(&tmp, &shat, &uhat);
     *    poly_invntt(&tmp);
     * 4. Subtract from vprime:
     *    poly_sub(&tmp, &tmp, &vprime).
     * 5. Decode polynomial coefficients back to 32-byte message:
     *    poly_tomsg(msg, &tmp).
     */
    (void)msg;
    (void)ct;
    (void)sk;
}
