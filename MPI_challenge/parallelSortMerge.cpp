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
    unsigned int chunkSize;
    unsigned int lasting = 0;
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
    MPI_Bcast(&chunkSize, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&lasting, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

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

        mergesNumber = displs.size() / 2;

        /** I need to give to the merge function also the index of the last element of the array in order to perform correctly a merge.
         ** I cannot insert it before because that would cause problems with scatter and gather **/
        displs.push_back(fullVec.size());

        std::cout << "mergesNumber initial: " << mergesNumber << std::endl;
    }

    MPI_Bcast(&mergesNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /** If the number of merges to perform is smaller than 2, the only main process suffices, all the others can return;
     *  If the number of merges requires more processes,  all those exceding the number can return; **/
    if ((mergesNumber < 2 && rank != 0) || (rank > mergesNumber - 1))
    {
        std::cout << "process " << rank << " exiting.." << std::endl;
        MPI_Finalize();
        return 0;
    }

    std::vector<int> newDispls;
    std::vector<int> newSend_counts;

    std::vector<int> src1, src2;

    newSend_counts.reserve(mergesNumber);
    int toAddIfNotEven = 0;

    while (mergesNumber >= 2)
    {

        if (rank == 0) // Computing the dataStructures needed for the scattering
        {
            newDispls.clear();
            newSend_counts.clear();

            for (int i = 0; i < displs.size(); i += 2)
                newDispls.push_back(displs[i]);

            for (int i = 0; i < newDispls.size() - 1; i++)
                newSend_counts.push_back(newDispls[i + 1] - newDispls[i]);

            chunkSize = newSend_counts[0];
            lasting = newSend_counts.back() - chunkSize;

            if (newDispls.back() == fullVec.size())
                newDispls.pop_back();
            if (fullVec.size() - newDispls.back() < chunkSize)
            {
                toAddIfNotEven = newDispls.back();
                newDispls.pop_back();
            }

            std::cout << "lasting: " << lasting << std::endl;
            for (int i = newSend_counts.size(); i < size; i++)
                newSend_counts.push_back(0); // you have to continue the counts otherwise the gather does not end;
        }

        // updating chunksize
        MPI_Bcast(&chunkSize, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
        MPI_Bcast(&lasting, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

        if (rank == mergesNumber - 1)
            chunkSize += lasting;

        smallVec.resize(chunkSize);

        if (rank == 0)
        {

            std::cout << "Scattering newDispls: ";
            for (int i = 0; i < newDispls.size(); i++)
            {
                std::cout << newDispls[i] << " ";
            }
            std::cout << "and newSend_counts: ";
            for (int i = 0; i < newSend_counts.size(); i++)
            {
                std::cout << newSend_counts[i] << " ";
            }

            std::cout << std::endl;
        }

        MPI_Scatterv(&fullVec[0], &newSend_counts[0], &newDispls[0], MPI_INT, &smallVec[0], chunkSize, MPI_INT, 0, MPI_COMM_WORLD);
        if (lasting == 0 || rank != mergesNumber - 1)
        {
            src1 = {smallVec.begin(), smallVec.begin() + smallVec.size() / 2};
            src2 = {smallVec.begin() + smallVec.size() / 2, smallVec.end()};
        }
        else
        {

            src1 = {smallVec.begin(), smallVec.begin() + ((smallVec.size() - lasting) / 2)};
            src2 = {smallVec.begin() + ((smallVec.size() - lasting) / 2), smallVec.end()};
        }

        merge(src1, src2, smallVec);

        MPI_Gatherv(&smallVec[0], chunkSize, MPI_INT, &fullVec[0], &newSend_counts[0], &newDispls[0], MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0)
        {
            displs = {newDispls.begin(), newDispls.end()};

            mergesNumber = displs.size() / 2;
            std::cout << "new merges number: " << mergesNumber << std::endl;

            for (int i = 0; i < displs.size(); i++)
                std::cout << displs[i] << " ";
        }

        MPI_Bcast(&mergesNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Last merge of the remainders (either two or three)parts
    if (rank == 0)
    {
        if (toAddIfNotEven != 0)
            displs.push_back(toAddIfNotEven);

        if (displs.back() != fullVec.size())
            displs.push_back(fullVec.size());

        while (displs.size() > 2)
        {
            std::cout << "Merging from " << displs[0] << " to " << displs[2] << std::endl;

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

    // std::cout << "process " << rank << " exiting.." << std::endl;
    MPI_Finalize();
    return 0;
}