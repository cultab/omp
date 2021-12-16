#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int **matrix;

void   print_mat(matrix, int, int);
matrix matlloc(int);
void   matfree(matrix, int);
int    load_matrix(matrix *, FILE *);

/* allocate memory for a matrix of size */
matrix matlloc(int size)
{
    /* allocate memory for matrix */
    matrix mat = malloc(sizeof(int *) * size);

    for (int i = 0; i < size; i++) {
        mat[i] = malloc(sizeof(int) * size);
    }
    return mat;
}

/* free a matrix of size */
void matfree(matrix mat, int size)
{
    /* deallocate memory for matrix */
    for (int i = 0; i < size; i++) {
        free(mat[i]);
    }
    free(mat);
}

/* print a matrix of size to stdout, using max_value to calculate padding for the elements */
void print_mat(matrix mat, int size, int max_value)
{
    int max    = (max_value * 9) + 1;
    int digits = 0;

    while (max != 0) {
        max = max / 10;
        digits++;
    }

    /* fprintf(stderr, "Matrix of %dx%d:\n", size, size); */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%*d ", digits + 1, mat[i][j]);
        }
        printf("\n");
    }
}

/* Load a matrix into address pointed by mat and return the matrix's size */
int load_matrix(matrix *mat, FILE *fp)
{
    int N = 0;
    fscanf(fp, "%d", &N);

    /* allocate memory for matrix */
    *mat = matlloc(N);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fscanf(fp, "%d", &(*mat)[i][j]); // pointers....
        }
    }

    return N;
}
