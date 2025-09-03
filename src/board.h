#ifndef __BOARD_H__
#define __BOARD_H__

#include "common.h"

const uint8_t WHITEKSIDE = 0x1;
const uint8_t WHITEQSIDE = 0x2;
const uint8_t BLACKKSIDE = 0x4;
const uint8_t BLACKQSIDE = 0x8;
const uint8_t WHITECASTLE = 0x3;
const uint8_t BLACKCASTLE = 0xC;

const uint16_t NO_EP_POSSIBLE = 0x8;

const bool MOVEGEN_CAPTURES = true;
const bool MOVEGEN_QUIETS = false;

struct PieceMoveInfo {
    int pieceID;
    int startSq;
    uint64_t legal;

    PieceMoveInfo() {}
    PieceMoveInfo(int _pieceID, int _startSq, uint64_t _legal) {
        pieceID = _pieceID;
        startSq = _startSq;
        legal = _legal;
    }
};

struct PieceMoveList {
    PieceMoveInfo arrayList[32];
    unsigned int length;
    unsigned int starts[5];

    PieceMoveList() {
        length = 0;
    }

    ~PieceMoveList() {}

    unsigned int size() {
        return length;
    }

    void add(PieceMoveInfo o) {
        arrayList[length] = o;
        length++;
    }

    PieceMoveInfo get(int i) {
        return arrayList[i];
    }
};

void initZobristTable();

class Board {
public:
    Board();
    Board(int *mailboxBoard, bool _whiteCanKCastle, bool _blackCanKCastle,
            bool _whiteCanQCastle, bool _blackCanQCastle,
            uint16_t _epCaptureFile, int _fiftyMoveCounter, int _moveNumber,
            int _playerToMove);
    ~Board();
    Board staticCopy();

    void doMove(Move m, int colour);
    bool doPseudoLegalMove(Move m, int colour);
    bool doHashMove(Move m, int colour);
    void doNullMove();
    void undoNullMove(uint16_t _epCaptureFile);

    PieceMoveList getPieceMoveList(int colour);
    MoveList getAllLegalMove(int colour);
    void getAllPseudoLegalMoves(MoveList &legalMoves, int colour);
    void getPseudoLegalQuiets(MoveList &quiets, int colour);
    void getPseudoLegalCaptures(MoveList &captures, int colour, bool includePromotions);
    void getPseudoLegalPromotions(MoveList &moves, int colour);
    void getPseudoLegalChecks(MoveList &checks, int colour);
    void getPseudoLegalCheckEscapes(MoveList &escapes, int colour);

    uint64_t getXRayPieceMap(int colour, int sq, int blockerColour,
            uint64_t blockerStart, uint64_t blockerEnd);
    uint64_t getAttackMap(int colour, int sq);
    uint64_t getAttackMap(int sq);
    int getPieceOnSquare(int colour, int sq);
    bool isCheckMove(int colour, int sq);
    uint64_t getRookXRays(int sq, uint64_t occ, uint64_t blockers);
    uint64_t getBishopXRays(int sq, uint64_t occ, uint64_t blockers);
    uint64_t getPinnedMap(int colour);

    bool isInCheck(int colour);
    bool isDraw();
    bool isInsufficientMaterial();
    void getCheckMaps(int colour, uint64_t *checkMaps);

    int getMaterial(int colour);

};
