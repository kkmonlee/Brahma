#include "common.h"
#include "bbinit.h"
#include <iostream>

int main() {
    std::cout << "Brahma Chess Engine - Build Test" << std::endl;
    
    uint64_t testBit = indexToBit(8);
    std::cout << "Bit 8: " << testBit << std::endl;
    
    int pos = bitScanForward(testBit);
    std::cout << "BSF of bit 8: " << pos << std::endl;
    
    int bits = count(0xFF);
    std::cout << "Popcount of 0xFF: " << bits << std::endl;
    
    std::cout << "Initializing magic tables..." << std::endl;
    initMagicTables(12345);
    
    std::cout << "Initializing in-between table..." << std::endl;
    initInBetweenTable();
    
    std::cout << "All systems initialized successfully!" << std::endl;
    
    return 0;
}