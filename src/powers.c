#include "../include/poly.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *file = fopen("roots.txt", "w");

    if(file == NULL){
        return -1;
    }
    
    int* roots = (int *)malloc(sizeof(int) * N);
    if (roots == NULL) {
        fclose(file);
        return -1;
    }

    int* head = roots;
    for(int i = 0; i < Q; i++){
        if(((i % Q) * (i % Q)) % Q == Q - 1){
            *head = i;
            printf("%d", i);
            head++;
        }
    }

    // Reset head back to the beginning of the array
    head = roots;
    
    // As guaranteed, this will loop exactly N times without overrunning memory
    while(head != 2 + roots){
        long long current_power = 1; // base^0 = 1
        
        for(int i = 0; i < N; i++){
            // Print the current power, safely bounded modulo 12289
            fprintf(file, "%lld, ", current_power);
            
            // Calculate the next power iteratively: (current_power * base) mod 12289
            current_power = (current_power * (*head)) % Q;
        }
        fprintf(file, "\n");
        head++;
    }

    free(roots);
    fclose(file);
    return 0;
}
