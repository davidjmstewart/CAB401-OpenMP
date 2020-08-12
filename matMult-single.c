#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define N 1000 // Matrix size will be N x N
#define NUM_THREADS 1
// COMPILE WITH: gcc matMult-single.c -o single
//     RUN WITH: ./single

int(*A)[N];
int(*B)[N];
int(*C)[N];

int main(int argc, char *argv[]) {

    struct timespec start, finish;
    double elapsed;


    A = malloc(sizeof(int[N][N]));
    B = malloc(sizeof(int[N][N]));
    C = malloc(sizeof(int[N][N])); // serial results

    // initialise our matrix with random data
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand();
            B[i][j] = rand();
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            C[i][j] = 0;
            for (int k = 0; k < N; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("SERIAL: Took %.9f seconds for %d elements \n", elapsed, N*N);

    return 0;
}