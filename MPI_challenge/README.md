Compiled with : mpicxx parallelSortMerge.cpp (or parallelSort.cpp) helpers.cpp -o mpi_mergesort  
Executed with : mpiexec -np ${num_processes} ./mpi_mergesort ${vector_length} 
If you want to spawn more processes than the actual available slots, overwrite 'my-hostfile' and set the number of slots equal to the number of processes you want and run the following command:
    mpiexec -np ${num_processes} --hostfile my-hostfile ./mpi_mergesort ${vector_length}