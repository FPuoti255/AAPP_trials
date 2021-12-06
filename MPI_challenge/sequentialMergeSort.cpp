#include "helpers.h"
#include <stdio.h>
#include <chrono>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Wrong number of parameters: " << argc - 1;
        return 1;
    }

    int vectorType = std::atoi(argv[1]);
    std::vector<int> fullVec;
    bool result = false;

    switch (vectorType)
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

    // We have to switch case in order to separate the vector-fetching phase from the actual mergesort phase
    auto initTime = std::chrono::high_resolution_clock::now();

    switch (vectorType)
    {
    case 1024:
        mergeSort(fullVec);
        result = checkEvenArray(fullVec);
        break;
    case 729:
        mergeSort(fullVec);
        result = checkOddArray(fullVec);
        break;
    case 997:
        mergeSort(fullVec);
        result = checkPrimeArray(fullVec);
        break;
    default:
        break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();

    std::cout << "Global vector check returned " << result << std::endl;

    std::cout << "Total time for merging: " << (std::chrono::duration_cast<std::chrono::microseconds>(endTime - initTime)).count()
              << " microseconds" << std::endl;

    return 0;
}