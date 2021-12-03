Compiled with : mpicxx primeMerge.cpp helpers.cpp -o mpi_mergesort 
Executed with : mpiexec -np ${num_threads} ./mpi_mergesort ${vector_length}   