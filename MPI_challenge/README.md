# MPI Merge sort #
### Compiled with :
     mpicxx parallelSortMerge.cpp (or parallelSort.cpp) helpers.cpp -o mpi_mergesort  
     mpicxx parallelSort.cpp helpers.cpp -o mpi_sort  

Optionally, adding -D debug will enable the statistics prints


### Executed with :
    mpiexec -np ${num_processes} ./mpi_mergesort ${vector_length} 
    mpiexec -np ${num_processes} ./mpi_sort ${vector_length} 

If you want to spawn more processes than the actual available slots on your pc, you need to:
- overwrite 'my-hostfile' and set the number of slots >= the number of processes you want
- add the command-line argument  --hostfile my-hostfile  to mpiexec