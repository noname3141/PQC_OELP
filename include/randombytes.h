#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

/*
 * Fills buffer x with xlen cryptographically secure random bytes from /dev/urandom.
 */
void randombytes(unsigned char *x, unsigned long long xlen);

#endif /* RANDOMBYTES_H */
