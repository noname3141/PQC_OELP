#ifndef VERIFY_H
#define VERIFY_H

#include <stddef.h>

/*
 * Constant-time byte array comparison.
 * Returns 0 if arrays are equal, non-zero otherwise.
 */
int verify(const unsigned char *a, const unsigned char *b, size_t len);

/*
 * Constant-time conditional move:
 * Copies x into r if b != 0, leaves r unchanged if b == 0.
 */
void cmov(unsigned char *r, const unsigned char *x, size_t len, unsigned char b);

#endif /* VERIFY_H */
