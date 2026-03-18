#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int dim = 100;
    double mat1[dim][dim];
    double mat2[dim][dim];
    double result[dim][dim];
    int* ptr;
    printf("%ld\n", sizeof(ptr));

    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            mat1[i][j] = ((double)rand()) / ((double)RAND_MAX + 1);
            mat2[i][j] = ((double)rand()) / ((double)RAND_MAX + 1);
        }
    }

    clock_t start = clock();
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            result[i][j] = 0;
            for (int k = 0; k < dim; k++) {
                result[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
    clock_t end = clock();

    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %f\n", total_time);
    
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         printf("%f ", mat1[i][j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         printf("%f ", mat2[i][j]);
    //     }
    //     printf("\n");
    // }
}