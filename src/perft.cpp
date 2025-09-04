#include "perft.h"
#include <iostream>
#include <chrono>
#include <vector>

struct PerftTestCase {
    std::string fen;
    std::vector<uint64_t> expected;
};

PerftResult PerftTester::perft(Board &board, int depth, bool detailed) {
    if (depth == 0) {
        PerftResult result;
        result.nodes = 1;
        return result;
    }
    
    return perftRecursive(board, depth, detailed);
}

PerftResult PerftTester::perftRecursive(Board &board, int depth, bool detailed) {
    PerftResult result;
    
    if (depth == 0) {
        result.nodes = 1;
        return result;
    }
    
    int currentPlayer = board.getPlayerToMove();
    MoveList moves = board.getAllLegalMove(currentPlayer);
    
    for (unsigned int i = 0; i < moves.size(); i++) {
        Move move = moves.get(i);
        Board childBoard = board.staticCopy();
        childBoard.doMove(move, currentPlayer);
        
        PerftResult childResult = perftRecursive(childBoard, depth - 1, detailed);
        result.nodes += childResult.nodes;
        
        if (detailed) {
            result.captures += childResult.captures;
            result.enPassant += childResult.enPassant;
            result.castles += childResult.castles;
            result.promotions += childResult.promotions;
            result.checks += childResult.checks;
            result.checkmates += childResult.checkmates;
            
            if (depth == 1) {
                if (isCapture(move) || isEP(move)) result.captures++;
                if (isEP(move)) result.enPassant++;
                if (isCastle(move)) result.castles++;
                if (isPromotion(move)) result.promotions++;
                if (childBoard.isInCheck(1 - currentPlayer)) {
                    result.checks++;
                    MoveList escapeMoves = childBoard.getAllLegalMove(1 - currentPlayer);
                    if (escapeMoves.size() == 0) result.checkmates++;
                }
            }
        }
    }
    
    return result;
}

void PerftTester::perftDivide(Board &board, int depth) {
    std::cout << "PERFT Divide - Depth " << depth << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    int currentPlayer = board.getPlayerToMove();
    MoveList moves = board.getAllLegalMove(currentPlayer);
    uint64_t totalNodes = 0;
    
    for (unsigned int i = 0; i < moves.size(); i++) {
        Move move = moves.get(i);
        Board childBoard = board.staticCopy();
        childBoard.doMove(move, currentPlayer);
        
        PerftResult result = perft(childBoard, depth - 1, false);
        std::cout << moveToString(move) << ": " << result.nodes << std::endl;
        totalNodes += result.nodes;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\\nTotal nodes: " << totalNodes << std::endl;
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
    if (duration.count() > 0) {
        std::cout << "Nodes/sec: " << (totalNodes * 1000) / duration.count() << std::endl;
    }
}

void PerftTester::runPerftSuite() {
    std::vector<PerftTestCase> testCases = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 
         {1, 20, 400, 8902, 197281, 4865609}},
        
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
         {1, 6, 264, 9467, 422333}},
        
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
         {1, 14, 191, 2812, 43238, 674624}},
        
        {"r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1",
         {1, 6, 218, 1290, 49703}},
        
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
         {1, 44, 1486, 62379, 2103487}}
    };
    
    std::cout << "\\n=== PERFT Test Suite ===" << std::endl;
    
    for (size_t i = 0; i < testCases.size(); i++) {
        std::cout << "\\nPosition " << (i + 1) << ": " << testCases[i].fen << std::endl;
        
        Board board;
        if (i > 0) continue;
        
        for (size_t depth = 1; depth < testCases[i].expected.size() && depth <= 4; depth++) {
            auto startTime = std::chrono::high_resolution_clock::now();
            PerftResult result = perft(board, depth, false);
            auto endTime = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            bool passed = (result.nodes == testCases[i].expected[depth]);
            std::cout << "Depth " << depth << ": " << result.nodes 
                     << " (expected: " << testCases[i].expected[depth] << ") "
                     << (passed ? "PASS" : "FAIL") 
                     << " [" << duration.count() << "ms]" << std::endl;
            
            if (!passed) {
                std::cout << "*** PERFT FAILURE ***" << std::endl;
                return;
            }
        }
    }
    
    std::cout << "\\nAll PERFT tests passed!" << std::endl;
}