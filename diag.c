#include "mat.h"

#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    /* default number of threads */
    int threads = 4;

    /* if given, set new number of threads */
    if (argc > 1) {
        threads = strtol(argv[1], NULL, 10);
    }

    printf("Using %d thread(s).\n", threads);

    /* load the matrix from stdin */
    matrix A;
    int    size = load_matrix(&A, stdin);

    printf("Matrix of size %d.\n", size);
    print_mat(A, size, 100);

    if (size < threads) {
        printf("t<n is not supported!\n\t%d thread(s) and matrix size of %d\n", threads, size);
        return 1;
    }

    /* assume that it is a strictly diagonally dominant matrix */
    bool sddm = true;

    double tic = omp_get_wtime();

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < size; i++) {
        int sum = 0;
        for (int j = 0; j < size; j++) {
            if (i != j) {
                sum += abs(A[i][j]);
            }
        }
        /*
         * If even one of the rows fails the test, it's not an sddm.
         * Also don't bother writing to it if we already know it's not sddm
         * we would like to break early if here but we can't break out of a parallel for :(
         */
        if (A[i][i] <= sum || !sddm) {
            #pragma omp atomic write
            sddm = false;
        }
    }

    double toc = omp_get_wtime();

    printf("%f seconds to test if A is an sddm.\n", toc - tic);

    /* exit if it's not an sddm */
    if (!sddm) {
        fprintf(stderr, "The matrix entered was not strictly diagonally dominant!\n");
        return 1;
    }

    tic = omp_get_wtime();

    /* find diagonal's max using reduction */
    int max = -INT_MAX;
    #pragma omp parallel for num_threads(threads) schedule(static) reduction(max : max)
    for (int i = 0; i < size; i++) {
        max = abs(A[i][i]) > max ? abs(A[i][i]) : max;
    }

    toc = omp_get_wtime();

    printf("%f seconds to find A's diagonal's max=%d.\n", toc - tic, max);

    /* alloc a new matrix */
    matrix B = matlloc(size);

    tic = omp_get_wtime();

    /* fill B as needed */
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i != j) {
                B[i][j] = max - A[i][j];
            } else {
                B[i][j] = max;
            }
        }
    }

    toc = omp_get_wtime();

    /* free matrix A */
    matfree(A, size);

    printf("%f seconds to create matrix B.\n", toc - tic);

    print_mat(B, size, max);

    tic = omp_get_wtime();

    /* find min using reduction */
    int min = INT_MAX;
    #pragma omp parallel for num_threads(threads) schedule(static) reduction(min : min)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            min = B[i][j] < min ? B[i][j] : min;
        }
    }

    toc = omp_get_wtime();

    printf("%f seconds to find B's min with reduction, min=%d.\n", toc - tic, min);

    tic = omp_get_wtime();

    /* find min using critical section */
    int min2 = INT_MAX;
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            #pragma omp critical
            {
                min2 = B[i][j] < min2 ? B[i][j] : min2;
            }
        }
    }

    toc = omp_get_wtime();

    printf("%f seconds to find B's min with critcal section, min=%d.\n", toc - tic, min2);

    int *min_arr = malloc(threads * sizeof(int));

    /* find min using the binary tree method */
    #pragma omp parallel num_threads(threads) shared(min_arr)
    {
        int tid       = omp_get_thread_num();
        int rows      = size / threads; // how many rows this thread gets
        int rem       = size % threads; // the number of rows that were not assigned to a thread
        int local_min = INT_MAX;
        /* if there exist rows that were not assigned, assign them to the last thread */
        if (rem != 0 && tid == threads - 1) {
            rows += rem;
        }
        /* printf("Thread(%d) rows=%d\n", tid, rows); */

        /* find the local minimun for the rows given to each thread */
        for (int i = tid; i < tid + rows; i++) {
            for (int j = 0; j < size; j++) {
                if (B[i][j] < local_min) {
                    /* printf("Thread(%d):new local min=%d\n", tid, local_min); */
                    local_min = B[i][j];
                }
            }
        }
        /* assign it to an array using the thread id as index */
        min_arr[tid] = local_min;

        /* fprintf(stderr, "Thread(%d):local min=%d\n", tid, local_min); */

        /* wait for all threads to find their local minimum */
        #pragma omp barrier

        /*
         *  In log2(number of threads) steps:
         *
         * j -> offset from thread's current element to the other thread's element it will compare it to,
         * i -> (always double of j's value) used to check if a tid has work to do
         * k -> counter upto log2(threads) steps to know when to end
         *
         *   T(n): thread with thread id = n
         *
         *         For 8 threads:
         *
         *         ⎧ T(0)  T(1)  T(2)   T(3)  T(4)   T(5)  T(6)   T(7)
         *         ⎪  \    /       \    /      \    /       \    /                                 .
         *         ⎪   \  /         \  /        \  /         \  /                                  .
         *         ⎪    \/           \/          \/           \/                                   .
         *         ⎪   T(0)         T(2)         T(4)         T(6)     j = 1, i = 2, k = 0         .
         *         ⎪      \        /                \        /                                     .
         * k steps ⎨       \      /                  \      /                                      .
         *         ⎪        \    /                    \    /                                       .
         *         ⎪         \  /                      \  /                                        .
         *         ⎪         T(0)                      T(4)            j = 2, i = 3, k = 1         .
         *         ⎪            \                     /
         *         ⎩             \---->   T(0)  <----/                 j = 4, i = 8, k = 2
         *
         *                                                               log2(threads) = 3
         */
        int j, i, k;
        for (j = 1, i = 2, k = 0; k < log2(threads); j *= 2, i *= 2, k++) {
            /* #pragma omp single */
            /* { */
            /*     fprintf(stderr, "min_arr: "); */
            /*     for (int g = 0; g < threads; g++) { */
            /*         fprintf(stderr, "%d ", min_arr[g]); */
            /*     } */
            /*     fprintf(stderr, "\n"); */
            /* } */
            if (tid % i == 0 && tid != threads - 1) {
                int res = min_arr[tid] < min_arr[tid + j] ? min_arr[tid] : min_arr[tid + j];
                /* fprintf(stderr, "Thread(%d): decided on %d between %d and %d\n", tid, res, min_arr[tid], min_arr[tid+j]); */
                min_arr[tid] = res;
            }
            #pragma omp barrier
        }
        /* printf("Thread(%d): done\n", tid); */
    }

    toc = omp_get_wtime();

    /* print_mat(B, size, max); */
    printf("%f seconds to find B's min with tree, min=%d.\n", toc - tic, min_arr[0]);

    /* free matrix B */
    matfree(B, size);

    return 0;
}
