#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>

#define N 200 // Matrix size will be N x N
#define NUM_THREADS  16
#define THREAD_RANGE 16 // Run for 1:THREAD_RANGE threads
#define NUM_AVERAGES 15 // take the average of 5 timings for each matrix size, and each number of threads

// COMPILE WITH: gcc -fopenmp matMult-comparison.c -g -o main -lpthread -lm
//     RUN WITH: ./main

typedef struct _thread_info
{
    int startRow;
    int endRow;
    int threadNum;
    struct timespec startTime;
    struct timespec endTime;
} thread_info_t;

int (*A)[N];
int (*B)[N];
int (*C)[N]; // store the serial implementation in this matrix
int (*P)[N]; // store the pthread implementation in this matrix
int (*O)[N]; // store the openmp implementation in this matrix

void *iPar(void *param) {
    thread_info_t *tInfo = (thread_info_t*)param;
    // printf("D: %d\n", tInfo->threadNum);
    

    int start = tInfo->startRow;
    int end = tInfo->endRow;
    for (int i = start; i < end; i++){
        for (int j = 0; j < N; j++){
            P[i][j] = 0;
            for (int k = 0; k < N; k++) {
            
                P[i][j] +=  A[i][k] * B[k][j];
            }
            
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &tInfo->endTime);
}

int main(int argc, char *argv[]){

    double pthreadTimings[NUM_THREADS];
    double openMPTimings[NUM_THREADS];


    struct timespec start, finish;
    double elapsed;

    A = malloc(sizeof(int[N][N]));
    B = malloc(sizeof(int[N][N]));
    C = malloc(sizeof(int[N][N])); // serial results
    P = malloc(sizeof(int[N][N])); // parallel results
    O = malloc(sizeof(int[N][N])); // openmp results

    bool doSerial = true;

    // initialise our matrix with random data
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            A[i][j] = rand();
            B[i][j] = rand();
        }
    }

    if (doSerial) {
        for (int r = 0; r < NUM_AVERAGES; r++) {
            clock_gettime(CLOCK_MONOTONIC, &start);

            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    C[i][j] = 0;
                    for (int k = 0; k < N; k++) {
                        C[i][j] += A[i][k] * B[k][j];
                    }
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &finish);
            elapsed += (finish.tv_sec - start.tv_sec);
            elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        }
        elapsed /= NUM_AVERAGES;
        printf("SERIAL: Took %.9f seconds for %d elements \n", elapsed, N*N);

    }

    pthread_t threads[THREAD_RANGE];
    thread_info_t threadInfo[THREAD_RANGE];

    for (int T = 1; T <= THREAD_RANGE; T++) {
        /* PARALLEL (P-THREADS) */
        threadInfo[T].threadNum = T;
        // printf("S: %d\n", T);

        // Presentation example: DEMONSTRATION OF MANUALLY DISTRIBUTING WORK - assigning contiguous rows to threads
        // TODO: Clean this section up, can be simplified
        if (N % T == 0) { // if the number of rows is evenly divisible by the number of threads
            int numRows = N / T;
            for (int i = 0; i < T; i++) {
                threadInfo[i].startRow = i * numRows;
                threadInfo[i].endRow   = i * numRows + numRows;
            }
        }
        else {  // if the number of rows in the matrix is NOT evenly divisible by the number of threads
            if (N <= T) {
                for (int i = 0; i < T - 1; i++) {
                    threadInfo[i].startRow = i;
                    threadInfo[i].endRow   = i + 1;
                }
            }
            else {
                int rowsRemaining = N;
                int threadsRemaining = T;
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

        int numIter = N <= T ? N : T;

        for (int r = 0; r < NUM_AVERAGES; r++){ 
            clock_gettime(CLOCK_MONOTONIC, &start);

            for (long t = 0; t < numIter; t++) {
                // printf("Timing for %d threads \n", t);
                int err = pthread_create(&threads[t], NULL, iPar, &threadInfo[t]);
                if (err)
                {
                    printf("ERROR; return code from pthread_create() is %d\n", err);
                    exit(-1);
                }
            }

            //Wait for the initial evaluation threads to complete
            for (long i = 0U; i < numIter; ++i) {
                // printf("F: %d\n", T);
                pthread_join(threads[i], NULL);
            }

            clock_gettime(CLOCK_MONOTONIC, &finish);
            elapsed = (finish.tv_sec - start.tv_sec);
            elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
            pthreadTimings[T - 1] += elapsed;

        }


    }

    printf("PTHREADS: ");
    for (int i = 0; i < THREAD_RANGE; i++) {
        pthreadTimings[i] /= NUM_AVERAGES;
        printf("%.9f ", pthreadTimings[i]);
    }
    
    printf("\n");


    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++)
        {
            assert(C[i][j] == P[i][j]);
        }
    }

    // PARALLEL (OPENMP)
    clock_gettime(CLOCK_MONOTONIC, &start);


    for (int r = 0; r < NUM_AVERAGES; r++) {

        for (int T = 1; T <= THREAD_RANGE; T++) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            omp_set_num_threads(T); // Use 4 threads for all consecutive parallel regions
            int i, j, k;
            #pragma omp parallel for private(i, j, k)  
            for (i = 0; i < N; i++) {
                for (j = 0; j < N; j++) {
                    O[i][j] = 0;
                    for (k = 0; k < N; k++) {
                        O[i][j] = O[i][j] + A[i][k] * B[k][j];
                    }
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &finish);
            elapsed = (finish.tv_sec - start.tv_sec);
            elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
            // printf("OPENMP PARALLEL: Took %.9f seconds for %d elements \n", elapsed, N * N);
            openMPTimings[T] += elapsed;

        }

    }

    printf("OPENMP: ");
    for (int T = 1; T <= THREAD_RANGE; T++) {
        openMPTimings[T]/=NUM_AVERAGES;
        printf("%.9f ", openMPTimings[T]);

    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            assert(O[i][j] == P[i][j]);
            assert(O[i][j] == C[i][j]);
        }
    }
    pthread_exit(NULL);
    return 0;
}