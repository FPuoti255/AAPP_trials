#include "helpers.h"
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>

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
    int chunkSize;
    int lasting = 0;
    std::vector<int> displs;
    std::vector<int> send_counts;

    int whichType = std::atoi(argv[1]);
    double initTime, endTime;

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

        initTime = MPI_Wtime();
    }

    // BroadCasting chunksize and lasting-elements
    MPI_Bcast(&chunkSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&lasting, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == size - 1)
        chunkSize += lasting;

    smallVec.resize(chunkSize);

    // BroadCasting the data to each thread
    MPI_Scatterv(&fullVec[0], &send_counts[0], &displs[0], MPI_INT, &smallVec[0], (chunkSize + lasting), MPI_INT, 0, MPI_COMM_WORLD);
    mergeSort(smallVec);

    MPI_Gatherv(&smallVec[0], chunkSize, MPI_INT, &fullVec[0], &send_counts[0], &displs[0], MPI_INT, 0, MPI_COMM_WORLD);

    /*************************************************************************************************************************************/
    /******************************* Merging-subroutine + print of final result **********************************************************/

    int mergesNumber;

    if (rank == 0)
    {
        for (int i = 1; i < displs.size(); i++)
            displs.erase(displs.begin() + i);

        mergesNumber = displs.size();

        if (displs.back() != fullVec.size())
            /** I need to give to the merge function also the index of the last element of the array in order to properly compute
             * how many elements each process must go through. **/
            displs.push_back(fullVec.size());
    }

    MPI_Bcast(&mergesNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /** If the number of merges to perform is smaller than 2, the only main-process suffices, all the others can return;
     *  If the number of merges requires more processes,  all those exceding the number can return; **/
    if ((mergesNumber < 2 && rank != 0) || (rank > mergesNumber - 1))
    {
        MPI_Finalize();
        return 0;
    }

    std::vector<int> src1, src2;

    while (mergesNumber >= 2)
    {

        if (rank == 0) // Computing the dataStructures needed for the scattering
        {

            send_counts.clear();

            for (int i = 0; i < displs.size() - 1; i++)
                send_counts.push_back(displs[i + 1] - displs[i]);

            chunkSize = send_counts[0];

            lasting = send_counts.back() - chunkSize;

            // I've to remove the last element (which is the fullVec.size(), otherwise the scatter and gather would not work)
            displs.pop_back();

            for (int i = send_counts.size(); i < size; i++)
                /** we have to specify for all the processes in the COMM_WORLD what the scatter and the gather will expect.
                 * For those already returned, it will be simply zero.*/
                send_counts.push_back(0);
        }

        // updating chunksize
        MPI_Bcast(&chunkSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&lasting, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == mergesNumber - 1)
            chunkSize += lasting;

        smallVec.resize(chunkSize);

        MPI_Scatterv(&fullVec[0], &send_counts[0], &displs[0], MPI_INT, &smallVec[0], chunkSize, MPI_INT, 0, MPI_COMM_WORLD);

        if (lasting == 0 || rank != mergesNumber - 1)
        {
            src1 = {smallVec.begin(), smallVec.begin() + smallVec.size() / 2};
            src2 = {smallVec.begin() + smallVec.size() / 2, smallVec.end()};
            merge(src1, src2, smallVec);
        }
        else // case of vector length or numer of chunks non dividible by 2
        {
            if (lasting > 0)
            {
                src1 = {smallVec.begin(), smallVec.begin() + (chunkSize - lasting) / 2};
                src2 = {smallVec.begin() + (chunkSize - lasting) / 2, smallVec.end()};
                merge(src1, src2, smallVec);
            }
            /**
             * if lastin<0 means that we have an odd number of merges to perform and thus the last chunk, which would be smaller than
             * the others, has been already ordered in the previous phase. It would be the main-process in the last phase to
             * merge it with the other chunks.**/
        }

        MPI_Gatherv(&smallVec[0], chunkSize, MPI_INT, &fullVec[0], &send_counts[0], &displs[0], MPI_INT, 0, MPI_COMM_WORLD);

        // now a merge has been performed, process with rank zero must prepare the data structures for the next iteration
        if (rank == 0)
        {

            for (int i = 1; i < displs.size(); i++)
                displs.erase(displs.begin() + i);

            mergesNumber = displs.size();

            if (displs.back() != fullVec.size())
                /** I need to give to the merge function also the index of the last element of the array in order to properly compute
                 * how many elements each process must go through. **/
                displs.push_back(fullVec.size());
        }

        MPI_Bcast(&mergesNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank > mergesNumber - 1)
            break;
    }

    // Last merge of the remainders (either two or three) parts
    if (rank == 0)
    {

        if (displs.back() != fullVec.size())
            displs.push_back(fullVec.size());

        if (displs.size() == 2)
        {
            displs.back() = (displs.back() - lasting) / 2;
            displs.push_back(fullVec.size());
        }

        while (displs.size() > 2)
        {

            src1 = {fullVec.begin() + displs[0], fullVec.begin() + displs[1]};
            src2 = {fullVec.begin() + displs[1], fullVec.begin() + displs[2]};
            merge(src1, src2, fullVec);

            displs.erase(displs.begin() + 1);
        }

        endTime = MPI_Wtime();

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

        std::cout << "Global vector check returned " << result << std::endl;
        std::cout << "Total time for merging: " << (endTime - initTime) * 1000 << " millisecs" << std::endl;
    }

    MPI_Finalize();
    return 0;
}