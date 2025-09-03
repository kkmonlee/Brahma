#include "common.h"
#include "bbinit.h"
#include "board.h"
#include <iostream>

int main() {
    std::cout << "Brahma Chess Engine - Build Test" << std::endl;
    
    uint64_t testBit = indexToBit(8);
    std::cout << "Bit 8: " << testBit << std::endl;
    
    int pos = bitScanForward(testBit);
    std::cout << "BSF of bit 8: " << pos << std::endl;
    
    int bits = count(0xFF);
    std::cout << "Popcount of 0xFF: " << bits << std::endl;
    
    std::cout << "Initializing systems..." << std::endl;
    initZobristTable();
    initMagicTables(12345);
    initInBetweenTable();
    
    std::cout << "Creating board..." << std::endl;
    Board board;
    
    std::cout << "White material: " << board.getMaterial(WHITE) << std::endl;
    std::cout << "Black material: " << board.getMaterial(BLACK) << std::endl;
    
    std::cout << "White in check: " << (board.isInCheck(WHITE) ? "Yes" : "No") << std::endl;
    std::cout << "Black in check: " << (board.isInCheck(BLACK) ? "Yes" : "No") << std::endl;
    
    std::cout << "All systems initialized successfully!" << std::endl;
    
    return 0;
}