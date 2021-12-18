#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>

int scan(unsigned int *in, unsigned int *out, unsigned int size)
{
    /* Implement this function */
}

bool check_result(unsigned int *res, unsigned int size) {
    /* Implement this function */
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
 * 
 * */

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Expected 1 output but got %d\n", argc - 1);
    return 0;
  }

  unsigned int vecSize = strtoumax(argv[1], NULL, 10);
  unsigned int initialVals[vecSize], finalVals[vecSize];

  /* the input vector of size vecSize will contain all the integers
   * from 0 to vecSize - 1
   */
  for (int ii = 0; ii <= vecSize; ii++) {
    initialVals[ii] = ii;
    finalVals[ii] = 0;
  }

  scan(initialVals, finalVals, vecSize);
  check(finalVals, vecSize);

  for (int ii = 0; ii < vecSize - 1; ii++) {
    printf("%d, ", finalVals[ii]);
  }

  printf("%d\n", finalVals[vecSize - 1]);

  return 0;
}