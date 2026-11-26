# Post-Quantum Cryptography (PQC): NewHope-1024 C Implementation & TLS 1.3 Deployment

This project provides an educational, modular C implementation of the **NewHope-1024 CCA-secure Key Encapsulation Mechanism (KEM)** with an end-to-end **TLS 1.3 Hybrid Deployment** over TCP with AES-256-GCM authenticated encryption.

---

## 1. Directory Structure

```
PQC_OELP/
├── certs/                 # Self-signed X.509 certificates and keys (SAN: localhost, 127.0.0.1)
│   ├── server.crt
│   └── server.key
├── deploy/                # Deployment and network handshake integration
│   ├── client.c           # TLS 1.3 + NewHope-1024 client application
│   ├── server.c           # TLS 1.3 + NewHope-1024 server application
│   └── handshake.c        # Protocol framing, key exchange, HKDF, AES-256-GCM AEAD
├── dummy_py/              # Python test scripts and assets (preserved intact)
├── include/               # Public API headers with full types, constants, prototypes
│   ├── cbd.h              # Centered Binomial Distribution sampling
│   ├── cpapke.h           # CPA-secure public key encryption primitives
│   ├── fips202.h          # Keccak / SHAKE-128 / SHAKE-256
│   ├── handshake.h        # Hybrid TLS 1.3 wire protocol framing and socket structs
│   ├── newhope.h          # CCA-secure KEM API (crypto_kem_*) and protocol wrappers
│   ├── ntt.h              # Number Theoretic Transform (NTT) forward/inverse
│   ├── params.h           # Ring parameters (N=1024, Q=12289, K=8, byte sizes)
│   ├── poly.h             # Polynomial representation and arithmetic
│   ├── randombytes.h      # CSPRNG interface (/dev/urandom)
│   ├── reduce.h           # Montgomery and Barrett reductions
│   └── verify.h           # Constant-time comparison (verify) and conditional move (cmov)
├── ref/                   # Reference implementation benchmark
├── src/                   # C source implementations with detailed TODO guides
│   ├── cbd.c              # CBD noise sampling
│   ├── cpapke.c           # CPA-PKE keygen, enc, dec
│   ├── fips202.c          # SHAKE implementation
│   ├── newhope.c          # CCA-KEM keygen, enc, dec (Targhi-Unruh transform)
│   ├── ntt.c              # NTT butterfly operations
│   ├── poly.c             # Polynomial serialization, compression, operations
│   ├── precomp.c          # Twiddle factors and precomputed roots
│   ├── randombytes.c      # System randomness
│   ├── reduce.c           # Montgomery and Barrett reduction
│   └── verify.c           # Constant-time routines
├── tests/                 # Unit tests and benchmarking suite
│   ├── benchmark.c        # Throughput & latency benchmarking
│   ├── test_cbd.c         # Centered binomial distribution unit test
│   ├── test_kat.c         # Known Answer & key consistency test
│   ├── test_newhope.c     # CCA-KEM key exchange and CCA security test
│   └── test_ntt.c         # Forward/Inverse NTT bit-exact roundtrip test
├── Makefile               # Complete build system
└── Readme.md              # Project documentation
```

---

## 2. Mathematical Parameters & Ring Definition

* **Polynomial Ring:** $R_q = \mathbb{Z}_q[X]/(X^N + 1)$
* **Ring Dimension:** $N = 1024$
* **Modulus:** $q = 12289$ (Prime where $q \equiv 1 \pmod{2N}$)
* **Noise Distribution Parameter:** $k = 8$ (Centered Binomial Distribution)
* **Montgomery Parameter:** $R = 2^{18} = 262144$, $q^{-1} = 12287$
* **Key & Ciphertext Sizes:**
  * Public Key: `1824` bytes (`1792` B poly + `32` B seed)
  * CPA Secret Key: `1792` bytes
  * CCA Secret Key: `3680` bytes (`1792` B sk + `1824` B pk + `32` B $H(pk)$ + `32` B $s$)
  * Ciphertext: `2208` bytes (`1792` B $u$ + `384` B compressed $v$ + `32` B confirm hash $d$)
  * Shared Secret: `32` bytes (256-bit symmetric key)

---

## 3. Function Implementation Guide (TODOs)

All functions in `src/` contain complete, working reference logic accompanied by detailed `/* TODO: ... */` blocks outlining the mathematical steps and algorithms:

1. **`src/reduce.c`**:
   * `montgomery_reduce(uint32_t a)`: Computes $a \cdot R^{-1} \pmod q$ where $R = 2^{18}$.
   * `barrett_reduce(uint16_t a)`: Computes $a \pmod q$ using precomputed multiplier $5/2^{16}$.
2. **`src/ntt.c`**:
   * `bitrev_vector(uint16_t *poly)`: In-place bit-reversal permutation for $N=1024$.
   * `mul_coefficients(uint16_t *poly, const uint16_t *factors)`: Pointwise twiddle factor multiplication.
   * `ntt(uint16_t *poly, const uint16_t *omegas)`: Radix-2 Cooley-Tukey NTT butterfly network.
3. **`src/cbd.c`**:
   * `cbd(poly *r, const unsigned char *buf)`: Generates noise from Hamming weight differences $\sum b_i - \sum b'_i$.
4. **`src/poly.c`**:
   * `poly_tobytes` / `poly_frombytes`: Packs and unrolls 4 14-bit coefficients into 7 bytes.
   * `poly_compress` / `poly_decompress`: Lossy compression from 14-bit down to 3-bit coefficients.
   * `poly_frommsg` / `poly_tomsg`: Redundant encoding/decoding of 256 message bits into $N=1024$ coefficients.
   * `poly_uniform`: SHAKE-128 rejection sampling into $\mathbb{Z}_q$.
   * `poly_sample`: Noise sampling using CBD with parameter $k=8$ via SHAKE-256.
   * `poly_mul_pointwise`: Pointwise multiplication in NTT frequency domain.
5. **`src/cpapke.c`**:
   * `cpapke_keypair`: Generates $b = a \cdot s + e$ with public seed.
   * `cpapke_enc`: Encrypts 32-byte message to $(u = a \cdot s' + e', v' = b \cdot s' + e'' + \text{msg})$.
   * `cpapke_dec`: Recovers message from $v' - s \cdot u$.
6. **`src/newhope.c`**:
   * `crypto_kem_keypair`: Packages CPA keypair, public key copy, $H(pk)$, and rejection seed $s$.
   * `crypto_kem_enc`: Derives shared secret and confirmation hash $d$ via Targhi-Unruh transform.
   * `crypto_kem_dec`: Decapsulates, re-encrypts, verifies in constant-time (`verify`), and conditionally falls back (`cmov`).

---

## 4. Building & Running

### Build All Targets
```bash
make
```

### Run Unit Tests
```bash
make test
```
Runs:
- `test_ntt`: Asserts bit-exact roundtrip $\text{INTT}(\text{NTT}(p)) = p \pmod q$.
- `test_cbd`: Checks noise bounds $\in [-8, 8]$ and mean centered at $0$.
- `test_kat`: Validates key serialization sizes and consistency.
- `test_newhope`: 10 iterations of keygen, enc, dec, and CCA tampering rejection.

### Run Performance Benchmarks
```bash
make bench
```

### Run End-to-End Post-Quantum Deployment
```bash
make run
```
This launches `server` on port 8443 in the background, connects `client`, performs:
1. Classical TLS 1.3 Handshake (ECDHE + RSA-2048 certificate validation).
2. NewHope-1024 Key Encapsulation (post-quantum hybrid layer over TLS tunnel).
3. HKDF-SHA256 directional symmetric key derivation (`pqc-c2s`, `pqc-s2c`).
4. Two-way AES-256-GCM AEAD encrypted application message transfer.
