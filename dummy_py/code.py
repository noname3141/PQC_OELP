import random
import hashlib
import os

Q = 12289
N = 1024

def mk_poly(N:int, Q:int) -> list[int]:
    a : int = []
    for i in range(0, N):
        a.append(random.randint(0, Q - 1))
    return a

def poly_add(a :list[int], b : list[int]) -> list[int]:
    return [(x + y) % Q for x, y in zip(a, b)]              #zip makes a list of pairs of each ele in both lists.
                                                            #truncates to smaller if size do not match
def poly_sub(a :list[int], b : list[int]) -> list[int]:
    return [(x - y) % Q for x, y in zip(a, b)]

def poly_mul(a :list[int], b : list[int], Q : int, N : int) -> list[int]:
    c = [0]*N
    for i in range(N):
        for j in range(N):
            if(i + j < N):
                    c[i + j] = (c[i + j] + a[i]*b[j]) % Q
            else:
                    c[i + j - N] = (c[i + j - N] + (-1)*a[i]*b[j]) % Q
    return c

def sample_cbd(N: int, k : int = 16,q : int = Q):
    
    num_bytes = (2 * k * N) // 8
    random_byte = os.urandom(num_bytes)

    coeff = []
    bit_idx = 0

    for _ in range(N):
        #count 1's in first k bits
        a = 0
        for _ in range(k):
            byte_val = random_byte[bit_idx // 8]
            a += (byte_val >> (bit_idx % 8)) & 1
            bit_idx += 1
        
        #count 1's in next k bits
        b = 0
        for _ in range(k):
            byte_val = random_byte[bit_idx // 8]
            b += (byte_val >> (bit_idx % 8)) & 1
            bit_idx += 1

        coeff.append((a - b) % Q)
    
    return coeff


## did not understand
def help_rec(v: int, q: int = Q) -> int:
    v = v % q
    bin_idx = ((4*v + q // 8) // q) % 4 #each quadrant expands to q len after 4*v, doing +q//8 makes each quadrant center around either 0 or q // 4 or q // 2 or 3q // 4 and // q extracts integer then % 4 wraps it to [0, 4).
    return bin_idx % 2

def rec(v:int, r:int, q:int = Q):
    shifted_v = (v - r *(q//4)) % q
    bit = ((2*shifted_v + q//4) // q) % 2
    return bit

def extract_key_bob(v:int, r:int, q:int = Q) ->int:
    return rec(v, r, q)

def bits_to_256bit_key(bit_list: list[int]) -> str:
    """Packs first 256 bits into 32 bytes and formats as hex."""
    byte_vals = bytearray()
    for i in range(0, 256, 8):
        byte = 0
        for b in range(8):
            byte |= (bit_list[i + b] << b)
        byte_vals.append(byte)
    return hashlib.sha256(byte_vals).hexdigest()

##

# 1. Public genrator 'a' (uniform random polynomial accross Z_q)
a = mk_poly(N, Q)


# 2. Alice generates her secret key (s) and error (e)
s = sample_cbd(N, k = 16, q = Q)
e = sample_cbd(N, k = 16, q = Q)

# 3. Bob generates her secret key (s1) and error (e1 and e2)
s1 = sample_cbd(N, k = 16, q = Q)
e1 = sample_cbd(N, k = 16, q = Q)
e2 = sample_cbd(N, k = 16, q = Q)

# 4. Alice and Bob make there public key
b = poly_add(e, poly_mul(a, s, Q, N))
u = poly_add(e1, poly_mul(a, s1, Q, N))

# 5. Alice and Bob shared secret
V_alice = poly_mul(u, s, Q, N)
V_bob = poly_add(poly_mul(b, s1, Q, N), e2)

dif = poly_sub(V_alice, V_bob)
s_dif = [x if x <= 12289 // 2 else x - 12289 for x in dif]


##did not understand
# Bob computes reconciliation hints (r) and extracts his shared bits
r_hints = [help_rec(val, Q) for val in V_bob[:256]]
bob_bits = [rec(val, r, Q) for val, r in zip(V_bob[:256], r_hints)]
bob = bits_to_256bit_key(bob_bits)

# 4. Bob sends (u, r_hints) over the wire to Alice
# Alice Decapsulation: v_alice = u*s
v_alice = poly_mul(u, s, Q, N)
alice_bits = [rec(val, r, Q) for val, r in zip(v_alice[:256], r_hints)]
alice = bits_to_256bit_key(alice_bits)

bob_key = hashlib.sha256(bob.encode('utf-8')).digest()
alice_key = hashlib.sha256(alice.encode('utf-8')).digest()
# 5. Output Verification
print("=== HANDSHAKE RESULT ===")
print(f"Alice Key: {alice_key.hex()}")
print(f"Bob Key:   {bob_key.hex()}")

assert alice_key == bob_key, "ERROR: Keys do not match!"
print("\n SUCCESS: Alice and Bob derived identical 256-bit symmetric keys!")
##
