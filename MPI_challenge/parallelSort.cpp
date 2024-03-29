#include "helpers.h"
#include <mpi.h>
#include <stdio.h>

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

    double initTime, totalTime = 0.0;

    std::vector<int> fullVec, smallVec;
    unsigned int chunkSize;
    unsigned int lasting = 0;
    std::vector<int> displs;
    std::vector<int> send_counts;

    int whichType = std::atoi(argv[1]);

    if (rank == 0)
    {
        switch (whichType)
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

        initTime = MPI_Wtime();

        chunkSize = fullVec.size() / size;
        lasting = fullVec.size() % size;

        // Preparing structures for MPI_Scatterv and MPI_Gatherv;
        displs.reserve(size);
        send_counts.reserve(size);

        int i = 0;
        for (; i < size; i++)
        {
            displs.push_back(chunkSize * i);
            send_counts.push_back(chunkSize);
        }
        send_counts[size - 1] += lasting;
    }

    // BroadCasting chunksize and lasting-elements
    MPI_Bcast(&chunkSize, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&lasting, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    if (rank == size - 1)
        chunkSize += lasting;

    smallVec.resize(chunkSize);

    // BroadCasting the data to each thread
    MPI_Scatterv(&fullVec[0], &send_counts[0], &displs[0], MPI_INT, &smallVec[0], (chunkSize + lasting), MPI_INT, 0, MPI_COMM_WORLD);
    mergeSort(smallVec);

    MPI_Gatherv(&smallVec[0], chunkSize, MPI_INT, &fullVec[0], &send_counts[0], &displs[0], MPI_INT, 0, MPI_COMM_WORLD);

#ifdef printWorkloads
    if (rank == 0)
    {

        std::cout << "_____________MPI SORTING PHASE______________" << std::endl;

        totalTime += MPI_Wtime() - initTime;

        for (int i = 0; i < send_counts.size(); i++)
            std::cout << "Process " << i << " sorted " << (double)send_counts[i] * 100.0 / (double)fullVec.size() << "\% of the array." << std::endl;

        std::cout << std::endl
                  << "_____________MPI MERGING PHASE______________" << std::endl
                  << "The main process will do all the work" << std::endl;

        initTime = MPI_Wtime();
    }
#endif
    /*************************************************************************************************************************************/
    /******************************* Merging-subroutine + print of final result **********************************************************/

    if (rank == 0)
    {
        std::vector<int> src1, src2;
        /**
         * I need to give to the merge function also the index of the last element of the array in order to perform correctly a merge.
         * I cannot insert it before because that would cause problems with scatter and gather
         **/
        displs.push_back(fullVec.size());

        while (displs.size() > 2)
        {
            src1 = {fullVec.begin() + displs[0], fullVec.begin() + displs[1]};
            src2 = {fullVec.begin() + displs[1], fullVec.begin() + displs[2]};

            merge(src1, src2, fullVec);

            displs.erase(displs.begin() + 1);
        }

        totalTime += MPI_Wtime() - initTime;

        bool result = false;
        switch (whichType)
        {
        case 1024:
            result = checkEvenArray(fullVec);
            break;
        case 729:
            result = checkOddArray(fullVec);
            break;
        case 997:
            result = checkPrimeArray(fullVec);
            break;
        default:
            break;
        }
        std::cout << std::endl
                  << "Global vector check returned " << result << std::endl;
        std::cout << "Total time for the algorithm: " << totalTime * 1000000 << " microsecs" << std::endl;
    }

    MPI_Finalize();
    return 0;
}