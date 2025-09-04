#ifndef __PERFT_H__
#define __PERFT_H__

#include "board.h"

struct PerftResult {
    uint64_t nodes;
    uint64_t captures;
    uint64_t enPassant;
    uint64_t castles;
    uint64_t promotions;
    uint64_t checks;
    uint64_t checkmates;
    
    PerftResult() : nodes(0), captures(0), enPassant(0), castles(0), 
                   promotions(0), checks(0), checkmates(0) {}
};

class PerftTester {
public:
    static PerftResult perft(Board &board, int depth, bool detailed = false);
    static void perftDivide(Board &board, int depth);
    static void runPerftSuite();
    
private:
    static PerftResult perftRecursive(Board &board, int depth, bool detailed);
    static bool isPerftPosition(const std::string &fen, int depth, uint64_t expected);
};

#endif