#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <omp.h>

#define N 1000 // Matrix size will be N x N
#define NUM_THREADS 10

// COMPILE WITH: gcc -fopenmp matMult-openmp.c -o mm-openmp 
//     RUN WITH: ./ mm-openmp 
int(*A)[N];
int(*B)[N];
int(*C)[N];


int main(int argc, char *argv[]) {

    struct timespec start, finish;
    double elapsed;

    A = malloc(sizeof(int[N][N]));
    B = malloc(sizeof(int[N][N]));
    C = malloc(sizeof(int[N][N]));
    // initialise our matrix with random data
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand();
            B[i][j] = rand();
        }
    }

    // PARALLEL (OPENMP)
    clock_gettime(CLOCK_MONOTONIC, &start);

    omp_set_num_threads(NUM_THREADS); // Use 4 threads for all consecutive parallel regions

    int i, j, k;

    #pragma omp parallel for private(i, j, k)  
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            C[i][j] = 0;
            for (k = 0; k < N; k++) {
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("OPENMP PARALLEL: Took %.9f seconds for %d elements \n", elapsed, N * N);
    return 0;
}