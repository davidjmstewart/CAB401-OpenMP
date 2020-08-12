#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>

#define N 1000 // Matrix size will be N x N
#define NUM_THREADS 1
// COMPILE WITH: gcc matMult-pthread.c -o mm-pthread -lpthread -lm
//     RUN WITH: ./mm-pthread

typedef struct _thread_info
{
    int startRow;
    int endRow;
} thread_info_t;


int (*A)[N];
int (*B)[N];
int (*C)[N];

void *iPar(void *param)
{
    thread_info_t *tInfo = (thread_info_t*)param;
   
    int start = tInfo->startRow;
    int end = tInfo->endRow;
    for (int i = start; i < end; i++){
        for (int j = 0; j < N; j++){
            long sum = 0;
            C[i][j] = 0;
            for (int k = 0; k < N; k++){
                C += A[i][k] * B[k][j];
            }
        }
    }
}

int main(int argc, char *argv[]){

    struct timespec start, finish;
    double elapsed;


    A = malloc(sizeof(int[N][N]));
    B = malloc(sizeof(int[N][N]));
    C = malloc(sizeof(int[N][N])); // serial results



    // initialise our matrix with random data
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            A[i][j] = rand();
            B[i][j] = rand();
        }
    }


    /* PARALLEL (P-THREADS) */
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_t threads[NUM_THREADS];
    thread_info_t threadInfo[NUM_THREADS];


    // Presentation example: DEMONSTRATION OF MANUALLY DISTRIBUTING WORK - assigning contiguous rows to threads
    // TODO: Clean this section up, can be simplified
    if (N % NUM_THREADS == 0) { // if the number of rows is evenly divisible by the number of threads
        int numRows = N / NUM_THREADS;
        for (int i = 0; i < NUM_THREADS; i++) {
            threadInfo[i].startRow = i * numRows;
            threadInfo[i].endRow   = i * numRows + numRows;
        }
    }
    else {  // if the number of rows in the matrix is NOT evenly divisible by the number of threads
        if (N <= NUM_THREADS) {
            for (int i = 0; i < N; i++) {
                threadInfo[i].startRow = i;
                threadInfo[i].endRow   = i + 1;
            }
        }
        else {
            int rowsRemaining = N;
            int threadsRemaining = NUM_THREADS;
            int numRows = 0;
            int thread = 0;
            while (rowsRemaining > 0) {
                numRows = ceil((float)rowsRemaining/threadsRemaining);
                threadInfo[thread].startRow = N - rowsRemaining;
                threadInfo[thread].endRow   = threadInfo[thread].startRow + numRows;
                rowsRemaining -= numRows;
                threadsRemaining--;
                thread++;
            }
        }
    }

    int numIter = N <= NUM_THREADS ? N : NUM_THREADS;

    for (long t = 0; t < numIter; t++) {
        int err = pthread_create(&threads[t], NULL, iPar, &threadInfo[t]);
        if (err)
        {
            printf("ERROR; return code from pthread_create() is %d\n", err);
            exit(-1);
        }
    }

    //Wait for the initial evaluation threads to complete
    for (long i = 0U; i < numIter; ++i) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("PARALLEL: Took %.9f seconds for %d elements \n", elapsed, N * N);

    return 0;
}