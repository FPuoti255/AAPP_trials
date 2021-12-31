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


template <typename T, typename C>
void scan(
     const T* in, // source data
     T* out,      // output data
     int size,    // size of source, output data sets
     C combine   // combine expression
 )
 {

     // upsweep (reduction)
     for (int stride = 1; stride < size; stride <<= 1) {
         #pragma omp parallel for schedule(guided)
         for (int i = 0; i < size; i += 2 * stride)
             out[2 * stride + i - 1] =
              combine(out[2 * stride + i - 1], out[stride + i - 1]);
     }

     // clear last element
     T last = out[size - 1];
     out[size - 1] = T(0);

     // downsweep
     for (int stride = size / 2; stride > 0; stride >>= 1) {
         #pragma omp parallel
         {
             #pragma omp for schedule(guided)
             for (int i = 0; i < size; i += 2 * stride) {
                 T temp = out[stride + i - 1];
                 out[stride + i - 1] = out[2 * stride + i - 1];
                 out[2 * stride + i - 1] =
                  combine(temp, out[2 * stride + i - 1]);
             }
         }
     }

     // shift left for inclusive scan and add last
     for (int i = 0; i < size - 1; i++)
         out[i] = out[i + 1];
     out[size - 1] = last;

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

 int main(int argc, char *argv[])
{

    if (argc != 2)
    {
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

    auto combine_function = [](int a, int b) {return a+b;}; 
    scan(initialVals, finalVals, vecSize, combine_function);

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
