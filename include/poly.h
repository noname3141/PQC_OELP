//Polynomial struct (int16_t coeffs[N]) & operations

#ifndef POLY_H
#define POLY_H

#include <stdio.h>
#include <stdint.h>
#include "params.h"

typedef struct Polynomial {
    int *coeff;
} poly;

void poly_add(const poly *A, const poly *B, poly *C);
void poly_sub(const poly *A, const poly *B, poly *C);
void poly_mul_native(const poly *A,const poly *B, poly *C);
void poly_ntt_mul(const poly *A, const poly *B, poly *C); //learn whats ntt?

void poly_tobytes(uint8_t *r, const poly* p);
void poly_frombytes(poly *p, const uint8_t *r);

#endif
