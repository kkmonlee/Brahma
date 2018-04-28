#include "bbinit.h"

static uint64_t mseed = 0, mstate = 0;

uint64_t magicRNG() {
    uint64_t y = ((mstate << 57) | (mseed << 57)) >> 1;
    mstate ^= mseed >> 17;
    mstate ^= mstate << 3;

    uint64_t temp = mseed;
    mseed = mstate;
    mstate = temp;

    return (y | (mseed ^ mstate)) >> 1;
}

const int NORTH_SOUTH_FILL = 0;
const int EAST_WEST_FILL = 1;
const int NE_SW_FILL = 9;
const int NW_SE_FILL = 7;

uint64_t fillRayRight(uint64_t rayPieces, uint64_t empty, int shift);
uint64_t fillRayLeft(uint64_t rayPieces, uint64_t empty, int shift);

static uint64_t ROOK_MASK[64];
static uint64_t BISHOP_MASK[64];

uint64_t *attackTable;

MagicInfo magicBishops[64];
MagicInfo magicRooks[64];

uint64_t inBetweenSqs[64][64];

uint64_t indexToMask64(int index, int nBits, uint64_t mask);
uint64_t ratt(int sq, uint64_t block);
uint64_t batt(int sq, uint64_t block);
int magicMap(uint64_t masked, uint64_t magic, int nBits);
uint64_t findMagic(int sq, int m, bool isBishop);

void initInBetweenTable() {
    for (int sq1 = 0; sq1 < 64; ++sq1) {
        for (sq2 = 0; sq2 < 64; ++sq2) {
            uint64_t imaginaryRook = ratt(sq1, indexToBit(sq2));
            if (imaginaryRook & indexToBit(sq2)) {
                uint64_t imaginaryRook2 = ratt(sq2, indexToBit(sq1));
                inBetweenSqs[sq1][sq2] = imaginaryRook & imaginaryRook2;
            } else {
                uint64_t imaginaryBishop = batt(sq1, indexToBit(sq2));
                if (imaginaryBishop & indexToBit(sq2)) {
                    uint64_t imaginaryBishop2 = batt(sq2, indexToBit(sq1));
                    inBetweenSqs[sq1][sq2] = imaginaryBishop & imaginaryBishop2;
                } else {
                    inBetweenSqs[sq1][sq2] = 0;
                }
            }
        }
    }
}

void initMagicTables(uint64_t seed) {
    mstate = 74036198046ULL;
    mseed = seed;
    
    for (int i = 0; i < 64; ++i) {
        uint64_t relevantBits = ((~FILES[0] & ~FILES[7]) | FILES[i & 7])
            & ((~RANKS[0] & ~RANKS[7]) | RANKS[i >> 3]);
        ROOM_MASK[i] = ratt(i, 0) & relevantBits;
        BISHOP_MASK[i] = batt(i, 0) & relevantBits;
    }

    attackTable = new uint64_t[107648];
    
    int runningPtrLoc = 0;

    for (int i = 0; i < 64; ++i) {
        uint64_t = *tableStart = attackTable;
        magicBishops[i].table = tableStart + runningPtrLoc;
        magicBishops[i].mask = BISHOP_MASK[i];
        magicBishops[i].magic = findMagic(i, NUM_BISHOP_BITS[i], true);
        magicBishops[i].shift = 64 - NUM_BISHOP_BITS[i];

        runningPtrLoc += 1 << NUM_BISHOP_BITS[i];
    }

    for (int i = 0; i < 64; ++i) {
        uint64_t *tableStart = attackTable;
        magicRooks[i].table = tableStart + runningPtrLoc;
        magicRooks[i].mask = ROOK_MASK[i];
        magicRooks[i].magic = findMagic(i, NUM_ROOK_BITS[i], false);
        magicRooks[i].shift = 64 - NUM_ROOK_BITS[i];
        runningPtrLoc += 1 << NUM_ROOK_BITS[i];
    }

    for (int sq = 0; sq < 64; ++sq) {
        int nBits = NUM_BISHOP_BITS[sq];
        uint64_t mask = BISHOP_MASK[sq];

        for (int i = 0; i < (1 << nBits); ++i) {
            uint64_t *attTableLoc = magicBishops[sq].table;
            uint64_t occ = indexToMask64(i, nBits, mask);
            uint64_t attSet = batt(sq, occ);
            int magicIndex = magicMap(occ, magicBishops[sq].magic, nBits);
            attTableLoc[magicIndex] = attSet;
        }
    }

    for (int sq = 0; sq < 64; ++sq) {
        int nBits = NUM_ROOK_BITS[sq];
        uint64_t mask = ROOK_MASK[sq];
        for (int i = 0; i < (1 << nBits); ++i) {
            uint64_t *attTableLoc
        }
    }


}
