//poly_add, poly_sub, byte packing/unpacking

#include "../include/poly.h"
#include "../include/params.h"
#include "../include/stdlib.h"
#include "../include/ntt.h"

void poly_add(const poly* A,const poly* B, poly* C){
    for(int i = 0; i < N; i++){
        C->coeff[i] = A->coeff[i] + B->coeff[i];
    }
}

void poly_sub(const poly* A,const poly* B, poly* C){
    for(int i = 0; i < N; i++){
        C->coeff[i] = A->coeff[i] - B->coeff[i];
    }
}

void poly_mul_native(const poly* A,const poly* B, poly* C){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(i+j < N){
                C->coeff[i + j] = C->coeff[i + j] + (A->coeff[i])*(B->coeff[j]);
            }
            else{
                C->coeff[i + j - N] = C->coeff[i + j - N] + -1*(A->coeff[i])*(B->coeff[j]);
            }
        }
    }
}

void poly_tobytes(uint8_t *r, const poly* p){
    int r_idx = 0;
    uint32_t buffer = 0;
    int bit_count = 0;
    for(int i = 0; i < N; i++){
        uint16_t cf = 16383 & p->coeff[i];

        buffer |= ((uint32_t)cf) << bit_count; //stores 14 bits of cf into buffer
        bit_count += 14; //increments bit counter by 14

        while(bit_count >= 8){
            r[r_idx++] = buffer & 0xFF; //pushes 8 bits of buffer into r then increment pointer
            buffer >>= 8; //shift buffer by 8
            bit_count -= 8; //decrement count
        }
    }
}

void poly_frombytes(poly *p, const uint8_t *r){
    int bit_count = 0;
    int r_idx = 0;
    uint32_t buffer = 0;
    for(int i = 0 ; i < N; i++){
        while(bit_count < 14){
            buffer |= ((uint32_t)r[r_idx++] << bit_count);
            bit_count += 8;
        }
        p->coeff[i] = (int)buffer & 0x3FFF;
        buffer >>= 14;
        bit_count -= 14;
    }
}

/*
void generate_poly(poly* p){
    int min = 0;
    int max = 12289;
    for(int i = 0; i < N; i++){
        int num = (rand() % (max - min + 1)) + min;
        p->coeff[i] = num;
    }
}

int main(){
    poly* p = malloc(sizeof(poly));
    p->coeff = malloc(N*sizeof(int));
    generate_poly(p);
    for(int i = 0; i < 1024; i++){
        printf("%d ", p->coeff[i]);
    }
    printf("\n");
    uint8_t* r = malloc(sizeof(uint8_t)*NEWHOPE_POLYBYTES);
    poly_tobytes(r, p);
    for(int i = 0; i < 1792; i++){
        printf("%02X ", r[i]);
    }
    printf("\n");
    poly_frombytes(p, r);
    for(int i = 0; i < 1024; i++){
        printf("%d ", p->coeff[i]);
    }
    free(p->coeff);
    free(p);
    free(r);
    return 0;
}
*/
