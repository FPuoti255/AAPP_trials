#include "helpers.h"
#include <mpi.h>

int main() {
    
    MPI_Init(nullptr, nullptr);
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<int> fullVec, smallVec;
    unsigned int chunkSize;
    bool checkRes;
    double initTime;

    if (rank == 0) {
        fullVec = getPrimeArray();
    }
    
    if (rank == 0) {
        std::cout << "Global vector check returned " << checkPrimeArray(fullVec) << " in " << MPI_Wtime() - initTime << "s" << std::endl;
    }

    MPI_Finalize();
    return 0;
}