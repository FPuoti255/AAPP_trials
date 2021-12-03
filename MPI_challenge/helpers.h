#include <vector>
#include <iostream>

void merge (const std::vector<int> &src1, const std::vector<int> &src2, std::vector<int> &vect);
void mergeSort(std::vector<int> &vect);

/* returns a vector of 1024 int vals */
std::vector<int> getEvenArray();
bool checkEvenArray(const std::vector<int> &vect);

/* returns a vector of 729 (3^6) int vals */
std::vector<int> getOddArray();
bool checkOddArray(const std::vector<int> &vect);

/* returns a vector of 997 (prime) int vals */
std::vector<int> getPrimeArray();
bool checkPrimeArray(const std::vector<int> &vect);