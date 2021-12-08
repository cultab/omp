#include <getopt.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mat.h"

#define HELP_STRING                                                                                                    \
    "make_mat [-h] [-d|-D] [-m] [-p|-P] N\n"                                                                           \
    "\n"                                                                                                               \
    "Print an NxN matrix to stdout.\n"                                                                                 \
    "\n"                                                                                                               \
    "positional arguments:\n"                                                                                          \
    "N, defaults to 4\n"                                                                                               \
    "\n"                                                                                                               \
    "optional arguments:\n"                                                                                            \
    "--help -h\n"                                                                                                      \
    "\tshow this message and exit\n"                                                                                   \
    "--sddm -d\n"                                                                                                      \
    "\tcreate a strictly diagonally dominant matrix, default behavior\n"                                               \
    "--not-sddm -D\n"                                                                                                  \
    "\tdon't create a strictly diagonally dominant matrix\n"                                                           \
    "--max-value -m\n"                                                                                                 \
    "\tset the value range for elements (expept diagonal) [-max, max], defaults to 10\n"                               \
    "--print-size -p\n"                                                                                                \
    "\tprint the size of the matrix, default behavior\n"                                                               \
    "--no-print-size -P\n"                                                                                             \
    "\tdon't print the size of the matrix\n"

static struct option long_options[] = {
    { "sddm",                no_argument, 0, 'd' },
    { "not-sddm",            no_argument, 0, 'D' },
    { "help",                no_argument, 0, 'h' },
    { "max-value",     required_argument, 0, 'm' },
    { "print-size",          no_argument, 0, 'p' },
    { "no-print-size",       no_argument, 0, 'P' },
    {           0,                     0, 0,  0  }
};

int main(int argc, char **argv)
{
    /* default max value for matrix elements */
    int max_value = 10;
    /* default size of matrix */
    int N = 4;

    /* create a strictly diagonally dominant matrix? */
    bool sddm = true;
    /* also print size of matrix */
    bool print_size = true;

    /* for benchmarking */
    /* double tic; */
    /* double toc; */

    /* parse args */
    char c;
    int  option_index = 0;
    while ((c = getopt_long(argc, argv, ":hdDm:pP", long_options, &option_index)) != -1) {
        switch (c) {
        case 'd':
            sddm = true;
            break;
        case 'D':
            sddm = false;
            break;
        case 'p':
            print_size = true;
            break;
        case 'P':
            print_size = false;
            break;
        case 'm':
            max_value = strtol(optarg, NULL, 10);
            break;
        case 'h':
            fprintf(stderr, "%s", HELP_STRING);
            exit(1);
        case ':':
            fprintf(stderr, "Option -%c requires an operand\n", optopt);
            break;
        case '?':
            fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
        }
    }

    /* handle possitional arguments */
    int begin_pos = optind;
    for (; optind < argc; optind++) {
        if (optind == begin_pos) {
            N = atoi(argv[optind]);
        } else {
            fprintf(stderr, "Ignoring positional argument: '%s'\n", argv[optind]);
        }
    }

    if (print_size) {
        printf("%d\n", N);
    }

    matrix mat = matlloc(N);

    /* tic = omp_get_wtime(); */

    /* seed for rand_r */
    unsigned int seed = rand();

    /* create a random matrix */
    #pragma omp parallel for schedule(static) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            /* the normal rand is NOT reentrant so performance tanks */
            int elem  = (rand_r(&seed) % (max_value * 2)) - max_value - 1;
            mat[i][j] = elem;
        }
    }

    /* toc = omp_get_wtime(); */

    /* fprintf(stderr, "%f\n", toc - tic); */

    /* make it a sddm */
    if (sddm) {
        for (int i = 0; i < N; i++) {
            int res = 0;
            #pragma omp parallel for schedule(static) reduction(+ : res)
            for (int j = 0; j < N; j++) {
                if (i != j) {
                    res += abs(mat[i][j]);
                }
            }
            mat[i][i] = res + 1;
        }
    }

    /* fprintf(stderr, "Matrix of %dx%d:\n", N, N); */
    /* print to stdout */
    print_mat(mat, N, max_value);

    return 0;
}

