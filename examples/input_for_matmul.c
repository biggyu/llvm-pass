#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));
    int dim = 100;
    double matd1[dim][dim];
    double matd2[dim][dim];
    double resultd[dim][dim];

    float matf1[dim][dim];
    float matf2[dim][dim];
    float resultdf[dim][dim];

    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            matd1[i][j] = ((double)rand()) / ((double)RAND_MAX + 1);
            matd2[i][j] = ((double)rand()) / ((double)RAND_MAX + 1);
        }
    }

    clock_t start = clock();
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            resultd[i][j] = 0;
            for (int k = 0; k < dim; k++) {
                resultd[i][j] += matd1[i][k] * matd2[k][j];
            }
        }
    }
    clock_t end = clock();

    double total_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Double type matmul execution time: %f\n", total_time);

    // for (int i = 0; i < dim; i++) {
    //     for (int j = 0; j < dim; j++) {
    //         matd1[i][j] = ((float)rand()) / ((float)RAND_MAX + 1);
    //         matd2[i][j] = ((float)rand()) / ((float)RAND_MAX + 1);
    //     }
    // }

    // start = clock();
    // for (int i = 0; i < dim; i++) {
    //     for (int j = 0; j < dim; j++) {
    //         resultd[i][j] = 0;
    //         for (int k = 0; k < dim; k++) {
    //             resultd[i][j] += matd1[i][k] * matd2[k][j];
    //         }
    //     }
    // }
    // end = clock();

    // total_time = (double)(end - start) / CLOCKS_PER_SEC;
    // printf("Float type matmul execution time: %f\n", total_time);
}