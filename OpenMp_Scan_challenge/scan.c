#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#define NT 4
#endif

bool check = 1;

void scan(unsigned int *in, unsigned int *out, unsigned int vecsize)
{

    /*
     * UP-SWEEP PHASE (reduction)
     * --------------------------
     * The first for loop sets the number of repetition of single stage (of the upsweep phase) to log_2(vecsize) times.
     *      Obviously, there should be a master thread which actually counts the number of iterations. The single stage is parallelized.
     *
     * The second for loop actually performs the single stage
     * 
     * level means at which level of the tree we are in the stage
     *
     */
    for (int level = 1; level < vecsize; level <<= 1)
    {
#pragma omp parallel for schedule(static)
        for (int i = 0; i < vecsize; i += 2 * level)
        {
            out[2 * level + i - 1] += out[level + i - 1];
        }
    }


    /*
     * DOWN-SWEEP PHASE
     * ----------------
     * Similarly as before, the main thread keeps the iteration counter and the single stage of the phase is parallelized.
     * And, yet, this time the the a right shift is performed to divide by 2. We are going dow in the level of the 'tree'
     * We can assume that the vecsize is always a power of 2
     * 
     * level means at which level of the tree we are in the stage
     *
     */
    for (int level = vecsize / 2; level > 1; level >>= 1)
    {
        int stage_stride = level / 2;
#pragma omp parallel for schedule(static)
        for(int i = level / 2; i < vecsize - level ; i += level)
        {
            out[(level + i -1)] += out[(level + i - 1) - stage_stride];
        }
    }
}

void check_result(unsigned int *res, unsigned int size)
{

#pragma omp parallel for schedule(static)
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

#ifdef _OPENMP
    omp_set_num_threads(NT);
#endif

    // vector initialization
#pragma omp parallel for schedule(guided)
    for (int ii = 0; ii < vecSize; ii++)
    {
        initialVals[ii] = ii;
        finalVals[ii] = ii;
    }

    scan(initialVals, finalVals, vecSize);

    check_result(finalVals, vecSize);
    printf("\nThe check returned: %d \n", check);
/*
    for (int ii = 0; ii < vecSize; ii++)
    {
        printf("%d ", finalVals[ii]);
    }
*/
    return 0;
}