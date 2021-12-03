#include "helpers.h"
#include <mpi.h>
#include <stdio.h>

#define vec_size = {1024, 729, 997}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Wrong number of parameters: " << argc - 1;
        return 1;
    }

    MPI_Init(nullptr, nullptr);
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<int> fullVec, smallVec;
    unsigned int chunkSize;
    unsigned int lasting = 0;
    int *displs;
    int *send_counts;

    if (rank == 0)
    {
        switch (std::atoi(argv[1]))
        {
        case 1024:
            fullVec = getEvenArray();
            break;
        case 729:
            fullVec = getOddArray();
            break;
        case 997:
            fullVec = getPrimeArray();
            break;
        default:
            break;
        }

        chunkSize = fullVec.size() / size;
        lasting = fullVec.size() % size;

        // Preparing structures for MPI_Scatterv and MPI_Gatherv;
        displs = (int *)malloc(size * sizeof(int));
        send_counts = (int *)malloc(size * sizeof(int));
        int i = 0;
        for (; i < size; i++)
        {
            displs[i] = chunkSize * i;
            send_counts[i] = chunkSize;
        }
        send_counts[size - 1] += lasting;
    }

    // BroadCasting chunksize and lasting-elements
    MPI_Bcast(&chunkSize, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&lasting, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    if (rank == size - 1)
        smallVec.resize(chunkSize + lasting);
    else
        smallVec.resize(chunkSize);

    // BroadCasting the data to each thread
    MPI_Scatterv(&fullVec[0], send_counts, displs, MPI_INT, &smallVec[0], (chunkSize + lasting), MPI_INT, 0, MPI_COMM_WORLD);
    mergeSort(smallVec);

    if (rank == 0)
        fullVec.clear();

    MPI_Gatherv(&smallVec[0], (chunkSize + lasting), MPI_INT, &fullVec[0] , send_counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    /**
     * TODO: implement the Merging routine;
    **/

    if (rank == 0)
    {
        std::cout << "Global vector check returned " << checkPrimeArray(fullVec) << " in " << MPI_Wtime() - initTime << "s" << std::endl;
    }

    MPI_Finalize();
    return 0;
}