#include <cstddef>
#include <iostream>
#include "cuckoo_hash.hpp"

int main(int argc, char *argv[]) {

#if DEBUG == 1
    std::cout << "Running in DEBUG mode." << std::endl;
#endif

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::cout << "Seed: " << seed << std::endl;
    CuckooHashTable c(2, 5, 1, seed);
    uint8_t val[1] = {4};
    uint8_t ele1[5] = {1, 2, 3, 4, 5};
    uint8_t ele2[5] = {1, 2, 8, 4, 6};
    uint8_t ele3[5] = {1, 2, 3, 4, 7};
    uint8_t ele4[5] = {1, 3, 3, 4, 8};
    uint8_t ele5[5] = {1, 2, 3, 4, 9};
    uint8_t ele6[5] = {1, 2, 5, 4, 10};
    uint8_t ele7[5] = {1, 3, 5, 4, 10};
    uint8_t ele8[5] = {1, 4, 5, 4, 10};
    uint8_t ele9[5] = {1, 5, 5, 4, 10};


    c.Print();
    c.Add(ele1, val);
    c.Print();
    c.Add(ele2, val);
    c.Print();
    c.Add(ele3, val);
    c.Print();
    c.Add(ele4, val);
    c.Print();
 
    c.Add(ele9, val);
    c.Print();   


}