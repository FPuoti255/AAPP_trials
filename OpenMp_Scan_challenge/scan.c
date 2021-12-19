#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define NT 4

unsigned int rows;
unsigned int cols;
unsigned int **tree;
bool check = 1;

void scan(unsigned int *in, unsigned int *out, unsigned int size)
{

    for (unsigned int i = 0; i < rows; i++)
    {
        tree[i] = (unsigned int *)malloc((size / (unsigned int)pow(2.0, i)) * sizeof(unsigned int));
    }

    tree[0] = out;

    // UP-SWEEP PHASE
    for (unsigned int r = 1; r < rows; r++)
    {
#pragma omp parallel for schedule(static) num_threads(NT)
        for (unsigned int c = 0; c < (size / (unsigned int)pow(2.0, r)); c++)
        {
            tree[r][c] = tree[r - 1][2 * c] + tree[r - 1][2 * c + 1];
        }
    }

    // DOWN-SWEEP PHASE
    for (int r = rows - 2; r >= 0; r--)
    {
#pragma omp parallel for schedule(static) num_threads(NT)
        for (unsigned int c = 1; c < (size / (unsigned int)pow(2.0, r)); c++)
        {
            if (c % 2 != 0)
            {
                tree[r][c] = tree[r + 1][c / 2];
            }
            else
            {
                tree[r][c] += tree[r + 1][c / 2 - 1];
            }
        }
    }
}

void check_result(unsigned int *res, unsigned int size)
{

#pragma omp parallel for schedule(static) num_threads(NT)
    for (int ii = 0; ii < size; ii++)
    {
        if (res[ii] != ii * (ii + 1) / 2)
            check = 0;
    }
}

/**
 *
 * Implement a scan function that adds to each element of the input vector all the previous elements in the same vector
 * using an upsweep + downsweep approach in order to efficiently compute the results in a parallel execution;
 *
 * The scan should assign to each element of the output vector the sum of the values present in the input vector
 * from the first one up to the element position (included). --> Inclusive Scan Pattern
 *
 * After the implementation of the scan is complete, implement a second function check_result that checks for correctness
 * of the result automatically: since values start from  0 , the value in position  n  should contain the value  (n*(n+1))/2.
 * If possible accelerate the check using parallelization as well.
 *
 * After obtaining a working parallel implementation,evaluate the speedup offered by each parallel construct in your solution
 * (tip: perform evaluation on a reasonably large vector);
 *
 * For simplicity in the implementation, you can assume that the size of the input vector will always be a power of  2 .
 *
 * Compilation commands:  ' gcc -fopenmp scan.c -o scan -lm '
 * -> -lm is necessary to tell the compiler to link the libraries. Including the *.h imports just the function declarations, but not their definitions !
 *
 * */

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Expected 1 output but got %d\n", argc - 1);
        return 0;
    }

    unsigned int vecSize = strtoumax(argv[1], NULL, 10);

    unsigned int initialVals[vecSize], finalVals[vecSize];

    for (int ii = 0; ii < vecSize; ii++)
    {
        initialVals[ii] = ii;
        finalVals[ii] = ii;
    }

    rows = (unsigned int)log2(vecSize) + 1;
    cols = vecSize;
    tree = (unsigned int **)malloc(rows * sizeof(unsigned int *));

    scan(initialVals, finalVals, vecSize);

    check_result(finalVals, vecSize);
    printf("\nThe check returned: %d \n", check);

    /*for (int ii = 0; ii < vecSize; ii++)
    {
        printf("%d ", finalVals[ii]);
    }*/

    return 0;
}