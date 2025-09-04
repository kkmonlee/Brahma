#include "common.h"
#include "bbinit.h"
#include "board.h"
#include "perft.h"
#include <iostream>

int main() {
    std::cout << "Brahma Chess Engine - Advanced Move Generation Test" << std::endl;
    
    std::cout << "Initializing systems..." << std::endl;
    initZobristTable();
    initMagicTables(12345);
    initInBetweenTable();
    
    std::cout << "Creating board..." << std::endl;
    Board board;
    
    std::cout << "\\n=== Board Status ===" << std::endl;
    std::cout << "White material: " << board.getMaterial(WHITE) << std::endl;
    std::cout << "Black material: " << board.getMaterial(BLACK) << std::endl;
    std::cout << "Player to move: " << (board.getPlayerToMove() == WHITE ? "White" : "Black") << std::endl;
    
    std::cout << "\\n=== Move Generation Test ===" << std::endl;
    MoveList allMoves = board.getAllLegalMove(WHITE);
    std::cout << "Legal moves for White: " << allMoves.size() << std::endl;
    
    std::cout << "\\n=== PERFT Validation ===" << std::endl;
    
    for (int depth = 1; depth <= 3; depth++) {
        auto startTime = std::chrono::high_resolution_clock::now();
        PerftResult result = PerftTester::perft(board, depth, true);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Depth " << depth << ": " << result.nodes << " nodes";
        if (depth > 1) {
            std::cout << " (" << result.captures << " captures, " 
                     << result.castles << " castles, " 
                     << result.promotions << " promotions, "
                     << result.enPassant << " en passant)";
        }
        std::cout << " [" << duration.count() << "ms]" << std::endl;
    }
    
    std::cout << "\\n=== Performance Test ===" << std::endl;
    std::cout << "Running PERFT divide for depth 2..." << std::endl;
    PerftTester::perftDivide(board, 2);
    
    std::cout << "\\nAdvanced move generation system operational!" << std::endl;
    
    return 0;
}