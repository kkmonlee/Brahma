# Brahma
Brahma is a UCI/Xboard-compliant chess engine written in C++14.

## Engine strength
*Brahma is currently in development. This section will be updated once the engine is in alpha.*

## Implementation
### Board representation
- Bitboards
- Fancy magic bitboards for a 4.5s PERFT 6 @ 3.81 GHz (no bulk counting)

### Search
- Lazy SMP (up to 128 threads)
- Iterative Deepening
- Fail-Hard Principal Variation Search
- Transposition Table
    - Zobrist Hashing
    - Two Bucket System
- Selectivity
    - Adaptive Null Move Pruning
    - Late Move Reductions
    - Futility Pruning
    - Razoring
    - Move Count Based Pruning
    - Check Extensions
    - Singular Extensions
- Quiescence Search
    - Captures
    - Queen Promotions
    - Checks on the first 3 plies
- Move Ordering
    - Internal Iterative Deepening
    - Static Exchange Evaluation
    - MVV/LVA to order captures
    - Killer Heuristic (for quiet moves)
    - History Heuristic (for quiet moves)

### Evaluation
- Evaluation Cache
- Piece-Square Tables
- King Safety
- Pawn Structure
- Mobility
- SWAR Tapered Evaluation
- Texel's Tuning Method
- Reinforcement learning
- Coordination descent
- Syzygy tablebase support
- Basic threat detection and pressure on weak pieces