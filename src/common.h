#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include <chrono>
#include <string>

#define USE_INLINE_ASM true

typedef uint64_t u64;

const int WHITE = 0;
const int BLACK = 1;
const int PAWNS = 0;
const int KNIGHTS = 1;
const int BISHOPS = 2;
const int ROOKS = 3;
const int QUEENS = 4;
const int KINGS = 5;

// Search score
const int MATE_SCORE = 32766;
const int INFTY = 32767;

// Other values
const int MAX_DEPTH = 127;
const int MAX_MOVES = 256;

// Timing
typedef std::chrono::high_resolution_clock ChessClock;
typedef std::chrono::high_resolution_clock::time_point ChessTime;

u64 getTimeElapsed(ChessTime startTime);

// Bitboard methods
int bitScanForward(u64 bb);
int bitScanReverse(u64 bb);
int count(u64 bb);
u64 flipAcrossRanks(u64 bb);
u64 indexToBit(int sq);

inline int relativeRank(int c, int r) {
    return (r ^ (7 * c));
}

// Moves (16-bit unsigned int)
typedef uint16_t Move;

const Move NULL_MOVE = 0;
const Move MOVE_DOUBLE_PAWN = 0x1;
const Move MOVE_EP = 0x5;
const Move MOVE_PROMO_N = 0x8;
const Move MOVE_PROMO_B = 0x9;
const Move MOVE_PROMO_R = 0xA;
const Move MOVE_PROMO_Q = 0xB;
const int PROMO[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4};

inline Move encodeMove(int startSq, int endSq) {
    return (endSq << 6) | startSq;
}

inline Move setCapture(Move m, bool isCapture) {
    return m | (isCapture << 14);
}

inline Move setCastle(Move m, bool isCastle) {
    return m | (isCastle << 13);
}

inline Move setFlags(Move m, Move f) {
    return m | (f << 12);
}

inline int getStartSq(Move m) {
    return (int) (m & 0x3F);
}

inline int getEndSq(Move m) {
    return (int) ((m >> 6) & 0x3F);
}

inline int getPromotion(Move m) {
    return PROMO[m >> 12];
}

inline bool isPromotion(Move m) {
    return (bool) (m >> 15);
}

inline bool isCapture(Move m) {
    return (bool) ((m >> 14) & 1);
}

inline bool isCastle(Move m) {
    return ((m >> 13) == 1);
}

inline bool isEP(Move m) {
    return ((m >> 12) == MOVE_EP);
}

inline Move getFlags(Move m) {
    return (m >> 12);
}

std::string moveToString(Move m);

template <class T> class SearchArrayList {
public:
    T arrayList[MAX_MOVES];
    unsigned int length;

    SearchArrayList() {
        length = 0;
    }

    ~SearchArrayList() {}

    unsigned int size() {
        return length;
    }

    void add(T o) {
        arrayList[length] = o;
        length++;
    }

    T get(int i) {
        return arrayList[i];
    }

    void set(int i, T o) {
        arrayList[i] = o;
    }

    T remove(int i) {
        T deleted = arrayList[i];
        for (unsigned int j = i; j < length - 1; j++) {
            arrayList[j] = arrayList[j + 1];
        }
        length--;
        return deleted;
    }

    void resize(int l) {
        length = l;
    }

    void swap(int i, int j) {
        T temp = arrayList[i];
        arrayList[i] = arrayList[j];
        arrayList[j] = temp;
    }

    void clear() {
        length = 0;
    }
};

typedef SearchArrayList<Move> MoveList;
typedef SearchArrayList<int> ScoreList;

#endif
