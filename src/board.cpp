#include "board.h"
#include "bbinit.h"
#include <cstring>

static uint64_t zobristTable[781];
static uint64_t zobristCastling[16];
static uint64_t zobristEP[8];
static uint64_t zobristSide;

void initZobristTable() {
    uint64_t seed = 1070372;
    for (int i = 0; i < 781; i++) {
        zobristTable[i] = seed;
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        seed = (seed << 32) | ((seed * 1103515245 + 12345) & 0x7fffffff);
    }
    
    for (int i = 0; i < 16; i++) {
        zobristCastling[i] = seed;
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        seed = (seed << 32) | ((seed * 1103515245 + 12345) & 0x7fffffff);
    }
    
    for (int i = 0; i < 8; i++) {
        zobristEP[i] = seed;
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        seed = (seed << 32) | ((seed * 1103515245 + 12345) & 0x7fffffff);
    }
    
    zobristSide = seed;
}

Board::Board() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            pieces[i][j] = 0;
        }
        allPieces[i] = 0;
    }
    
    pieces[WHITE][PAWNS] = 0x000000000000FF00ULL;
    pieces[WHITE][ROOKS] = 0x0000000000000081ULL;
    pieces[WHITE][KNIGHTS] = 0x0000000000000042ULL;
    pieces[WHITE][BISHOPS] = 0x0000000000000024ULL;
    pieces[WHITE][QUEENS] = 0x0000000000000008ULL;
    pieces[WHITE][KINGS] = 0x0000000000000010ULL;
    
    pieces[BLACK][PAWNS] = 0x00FF000000000000ULL;
    pieces[BLACK][ROOKS] = 0x8100000000000000ULL;
    pieces[BLACK][KNIGHTS] = 0x4200000000000000ULL;
    pieces[BLACK][BISHOPS] = 0x2400000000000000ULL;
    pieces[BLACK][QUEENS] = 0x0800000000000000ULL;
    pieces[BLACK][KINGS] = 0x1000000000000000ULL;
    
    for (int i = 0; i < 2; i++) {
        allPieces[i] = 0;
        for (int j = 0; j < 6; j++) {
            allPieces[i] |= pieces[i][j];
        }
    }
    
    castlingRights = WHITECASTLE | BLACKCASTLE;
    epCaptureFile = NO_EP_POSSIBLE;
    fiftyMoveCounter = 0;
    moveNumber = 1;
    playerToMove = WHITE;
    zobristKey = calculateZobristKey();
}

Board::Board(int *mailboxBoard, bool _whiteCanKCastle, bool _blackCanKCastle,
        bool _whiteCanQCastle, bool _blackCanQCastle,
        uint16_t _epCaptureFile, int _fiftyMoveCounter, int _moveNumber,
        int _playerToMove) {
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            pieces[i][j] = 0;
        }
        allPieces[i] = 0;
    }
    
    for (int sq = 0; sq < 64; sq++) {
        int piece = mailboxBoard[sq];
        if (piece != -1) {
            int color = piece / 6;
            int pieceType = piece % 6;
            pieces[color][pieceType] |= indexToBit(sq);
            allPieces[color] |= indexToBit(sq);
        }
    }
    
    castlingRights = 0;
    if (_whiteCanKCastle) castlingRights |= WHITEKSIDE;
    if (_whiteCanQCastle) castlingRights |= WHITEQSIDE;
    if (_blackCanKCastle) castlingRights |= BLACKKSIDE;
    if (_blackCanQCastle) castlingRights |= BLACKQSIDE;
    
    epCaptureFile = _epCaptureFile;
    fiftyMoveCounter = _fiftyMoveCounter;
    moveNumber = _moveNumber;
    playerToMove = _playerToMove;
    zobristKey = calculateZobristKey();
}

Board::~Board() {}

Board Board::staticCopy() {
    Board copy;
    memcpy(copy.pieces, pieces, sizeof(pieces));
    memcpy(copy.allPieces, allPieces, sizeof(allPieces));
    copy.castlingRights = castlingRights;
    copy.epCaptureFile = epCaptureFile;
    copy.fiftyMoveCounter = fiftyMoveCounter;
    copy.moveNumber = moveNumber;
    copy.playerToMove = playerToMove;
    copy.zobristKey = zobristKey;
    return copy;
}

uint64_t Board::calculateZobristKey() {
    uint64_t key = 0;
    
    for (int color = 0; color < 2; color++) {
        for (int piece = 0; piece < 6; piece++) {
            uint64_t bb = pieces[color][piece];
            while (bb) {
                int sq = bitScanForward(bb);
                key ^= zobristTable[color * 6 * 64 + piece * 64 + sq];
                bb &= bb - 1;
            }
        }
    }
    
    key ^= zobristCastling[castlingRights];
    
    if (epCaptureFile != NO_EP_POSSIBLE) {
        key ^= zobristEP[epCaptureFile];
    }
    
    if (playerToMove == BLACK) {
        key ^= zobristSide;
    }
    
    return key;
}

void Board::doMove(Move m, int colour) {
    int startSq = getStartSq(m);
    int endSq = getEndSq(m);
    uint64_t startBit = indexToBit(startSq);
    uint64_t endBit = indexToBit(endSq);
    
    int pieceType = -1;
    for (int p = 0; p < 6; p++) {
        if (pieces[colour][p] & startBit) {
            pieceType = p;
            break;
        }
    }
    
    pieces[colour][pieceType] &= ~startBit;
    pieces[colour][pieceType] |= endBit;
    allPieces[colour] &= ~startBit;
    allPieces[colour] |= endBit;
    
    if (isCapture(m)) {
        int opponent = 1 - colour;
        for (int p = 0; p < 6; p++) {
            if (pieces[opponent][p] & endBit) {
                pieces[opponent][p] &= ~endBit;
                allPieces[opponent] &= ~endBit;
                break;
            }
        }
    }
    
    if (isCastle(m)) {
        if (endSq == 6) {
            pieces[WHITE][ROOKS] &= ~indexToBit(7);
            pieces[WHITE][ROOKS] |= indexToBit(5);
            allPieces[WHITE] &= ~indexToBit(7);
            allPieces[WHITE] |= indexToBit(5);
        } else if (endSq == 2) {
            pieces[WHITE][ROOKS] &= ~indexToBit(0);
            pieces[WHITE][ROOKS] |= indexToBit(3);
            allPieces[WHITE] &= ~indexToBit(0);
            allPieces[WHITE] |= indexToBit(3);
        } else if (endSq == 62) {
            pieces[BLACK][ROOKS] &= ~indexToBit(63);
            pieces[BLACK][ROOKS] |= indexToBit(61);
            allPieces[BLACK] &= ~indexToBit(63);
            allPieces[BLACK] |= indexToBit(61);
        } else if (endSq == 58) {
            pieces[BLACK][ROOKS] &= ~indexToBit(56);
            pieces[BLACK][ROOKS] |= indexToBit(59);
            allPieces[BLACK] &= ~indexToBit(56);
            allPieces[BLACK] |= indexToBit(59);
        }
    }
    
    if (isPromotion(m)) {
        int promoType = getPromotion(m);
        pieces[colour][PAWNS] &= ~endBit;
        pieces[colour][promoType] |= endBit;
    }
    
    if (isEP(m)) {
        int captureRank = (colour == WHITE) ? 4 : 3;
        int captureSq = captureRank * 8 + (endSq & 7);
        uint64_t captureBit = indexToBit(captureSq);
        pieces[1 - colour][PAWNS] &= ~captureBit;
        allPieces[1 - colour] &= ~captureBit;
    }
    
    if (pieceType == KINGS) {
        castlingRights &= (colour == WHITE) ? ~WHITECASTLE : ~BLACKCASTLE;
    }
    if (pieceType == ROOKS) {
        if (startSq == 0) castlingRights &= ~WHITEQSIDE;
        if (startSq == 7) castlingRights &= ~WHITEKSIDE;
        if (startSq == 56) castlingRights &= ~BLACKQSIDE;
        if (startSq == 63) castlingRights &= ~BLACKKSIDE;
    }
    
    epCaptureFile = NO_EP_POSSIBLE;
    if (pieceType == PAWNS && abs(endSq - startSq) == 16) {
        epCaptureFile = endSq & 7;
    }
    
    if (pieceType == PAWNS || isCapture(m)) {
        fiftyMoveCounter = 0;
    } else {
        fiftyMoveCounter++;
    }
    
    if (colour == BLACK) {
        moveNumber++;
    }
    
    playerToMove = 1 - playerToMove;
    zobristKey = calculateZobristKey();
}

bool Board::isInCheck(int colour) {
    uint64_t kingBB = pieces[colour][KINGS];
    if (!kingBB) return false;
    
    int kingSq = bitScanForward(kingBB);
    return isSquareAttacked(kingSq, 1 - colour);
}

bool Board::isSquareAttacked(int sq, int byColour) {
    uint64_t occ = allPieces[WHITE] | allPieces[BLACK];
    
    uint64_t pawnAttacks = getPawnAttacks(sq, 1 - byColour);
    if (pawnAttacks & pieces[byColour][PAWNS]) return true;
    
    if (KNIGHTMOVES[sq] & pieces[byColour][KNIGHTS]) return true;
    
    if (KINGMOVES[sq] & pieces[byColour][KINGS]) return true;
    
    uint64_t rookAttacks = getRookAttacks(sq, occ);
    if (rookAttacks & (pieces[byColour][ROOKS] | pieces[byColour][QUEENS])) return true;
    
    uint64_t bishopAttacks = getBishopAttacks(sq, occ);
    if (bishopAttacks & (pieces[byColour][BISHOPS] | pieces[byColour][QUEENS])) return true;
    
    return false;
}

uint64_t Board::getPawnAttacks(int sq, int colour) {
    if (colour == WHITE) {
        uint64_t attacks = 0;
        if (sq >= 8 && (sq & 7) != 0) attacks |= indexToBit(sq + 7);
        if (sq >= 8 && (sq & 7) != 7) attacks |= indexToBit(sq + 9);
        return attacks;
    } else {
        uint64_t attacks = 0;
        if (sq < 56 && (sq & 7) != 0) attacks |= indexToBit(sq - 9);
        if (sq < 56 && (sq & 7) != 7) attacks |= indexToBit(sq - 7);
        return attacks;
    }
}

uint64_t Board::getRookAttacks(int sq, uint64_t occ) {
    return ratt(sq, occ);
}

uint64_t Board::getBishopAttacks(int sq, uint64_t occ) {
    return batt(sq, occ);
}

void Board::getAllPseudoLegalMoves(MoveList &moves, int colour) {
    moves.clear();
    uint64_t occupied = allPieces[WHITE] | allPieces[BLACK];
    uint64_t friendly = allPieces[colour];
    uint64_t enemy = allPieces[1 - colour];
    
    generatePawnMoves(moves, colour, occupied, enemy);
    
    generateKnightMoves(moves, colour, friendly);
    
    generateBishopMoves(moves, colour, occupied, friendly);
    
    generateRookMoves(moves, colour, occupied, friendly);
    
    generateQueenMoves(moves, colour, occupied, friendly);
    
    generateKingMoves(moves, colour, occupied, friendly);
}

void Board::generatePawnMoves(MoveList &moves, int colour, uint64_t occupied, uint64_t enemy) {
    uint64_t pawns = pieces[colour][PAWNS];
    int forward = (colour == WHITE) ? 8 : -8;
    int promoRank = (colour == WHITE) ? 6 : 1;
    
    uint64_t singlePushTargets = (colour == WHITE) ? 
        (pawns << 8) & ~occupied : (pawns >> 8) & ~occupied;
    
    while (singlePushTargets) {
        int to = bitScanForward(singlePushTargets);
        int from = to - forward;
        
        if ((to >> 3) == promoRank + forward/8) {
            moves.add(setFlags(encodeMove(from, to), MOVE_PROMO_Q));
            moves.add(setFlags(encodeMove(from, to), MOVE_PROMO_R));
            moves.add(setFlags(encodeMove(from, to), MOVE_PROMO_B));
            moves.add(setFlags(encodeMove(from, to), MOVE_PROMO_N));
        } else {
            moves.add(encodeMove(from, to));
        }
        singlePushTargets &= singlePushTargets - 1;
    }
    
    uint64_t startingPawns = pawns & ((colour == WHITE) ? RANKS[1] : RANKS[6]);
    uint64_t doublePushTargets = (colour == WHITE) ? 
        (startingPawns << 16) & ~occupied : (startingPawns >> 16) & ~occupied;
    
    while (doublePushTargets) {
        int to = bitScanForward(doublePushTargets);
        int from = to - 2 * forward;
        moves.add(setFlags(encodeMove(from, to), MOVE_DOUBLE_PAWN));
        doublePushTargets &= doublePushTargets - 1;
    }
    
    uint64_t leftCaptures = (colour == WHITE) ? 
        ((pawns & NOTA) << 7) & enemy : ((pawns & NOTA) >> 9) & enemy;
    uint64_t rightCaptures = (colour == WHITE) ? 
        ((pawns & NOTH) << 9) & enemy : ((pawns & NOTH) >> 7) & enemy;
    
    generatePawnCaptures(moves, leftCaptures, colour, (colour == WHITE) ? -7 : 9, promoRank + forward/8);
    generatePawnCaptures(moves, rightCaptures, colour, (colour == WHITE) ? -9 : 7, promoRank + forward/8);
    
    if (epCaptureFile != NO_EP_POSSIBLE) {
        generateEnPassantMoves(moves, colour);
    }
}

void Board::generatePawnCaptures(MoveList &moves, uint64_t captures, int, int fromOffset, int promoRank) {
    while (captures) {
        int to = bitScanForward(captures);
        int from = to + fromOffset;
        
        if ((to >> 3) == promoRank) {
            moves.add(setCapture(setFlags(encodeMove(from, to), MOVE_PROMO_Q), true));
            moves.add(setCapture(setFlags(encodeMove(from, to), MOVE_PROMO_R), true));
            moves.add(setCapture(setFlags(encodeMove(from, to), MOVE_PROMO_B), true));
            moves.add(setCapture(setFlags(encodeMove(from, to), MOVE_PROMO_N), true));
        } else {
            moves.add(setCapture(encodeMove(from, to), true));
        }
        captures &= captures - 1;
    }
}

void Board::generateEnPassantMoves(MoveList &moves, int colour) {
    int epRank = (colour == WHITE) ? 4 : 3;
    int epSquare = epRank * 8 + epCaptureFile;
    
    uint64_t epCandidates = pieces[colour][PAWNS] & RANKS[epRank];
    if (epCandidates & indexToBit(epSquare - 1) && (epSquare & 7) != 0) {
        moves.add(setFlags(encodeMove(epSquare - 1, epSquare + ((colour == WHITE) ? 8 : -8)), MOVE_EP));
    }
    if (epCandidates & indexToBit(epSquare + 1) && (epSquare & 7) != 7) {
        moves.add(setFlags(encodeMove(epSquare + 1, epSquare + ((colour == WHITE) ? 8 : -8)), MOVE_EP));
    }
}

template<int PieceType>
void Board::generatePieceMoves(MoveList &moves, int colour, uint64_t occupied, uint64_t friendly) {
    uint64_t pieces_bb = pieces[colour][PieceType];
    
    while (pieces_bb) {
        int from = bitScanForward(pieces_bb);
        uint64_t attacks = 0;
        
        switch (PieceType) {
            case KNIGHTS:
                attacks = KNIGHTMOVES[from];
                break;
            case KINGS:
                attacks = KINGMOVES[from];
                break;
            case BISHOPS:
                attacks = getBishopAttacks(from, occupied);
                break;
            case ROOKS:
                attacks = getRookAttacks(from, occupied);
                break;
            case QUEENS:
                attacks = getRookAttacks(from, occupied) | getBishopAttacks(from, occupied);
                break;
        }
        
        attacks &= ~friendly;
        
        while (attacks) {
            int to = bitScanForward(attacks);
            Move move = encodeMove(from, to);
            if (allPieces[1 - colour] & indexToBit(to)) {
                move = setCapture(move, true);
            }
            moves.add(move);
            attacks &= attacks - 1;
        }
        
        pieces_bb &= pieces_bb - 1;
    }
}

void Board::generateKnightMoves(MoveList &moves, int colour, uint64_t friendly) {
    generatePieceMoves<KNIGHTS>(moves, colour, 0, friendly);
}

void Board::generateBishopMoves(MoveList &moves, int colour, uint64_t occupied, uint64_t friendly) {
    generatePieceMoves<BISHOPS>(moves, colour, occupied, friendly);
}

void Board::generateRookMoves(MoveList &moves, int colour, uint64_t occupied, uint64_t friendly) {
    generatePieceMoves<ROOKS>(moves, colour, occupied, friendly);
}

void Board::generateQueenMoves(MoveList &moves, int colour, uint64_t occupied, uint64_t friendly) {
    generatePieceMoves<QUEENS>(moves, colour, occupied, friendly);
}

void Board::generateKingMoves(MoveList &moves, int colour, uint64_t, uint64_t friendly) {
    generatePieceMoves<KINGS>(moves, colour, 0, friendly);
    
    generateCastlingMoves(moves, colour);
}

void Board::generateCastlingMoves(MoveList &moves, int colour) {
    if (isInCheck(colour)) return;
    
    uint64_t occupied = allPieces[WHITE] | allPieces[BLACK];
    int rank = (colour == WHITE) ? 0 : 56;
    
    if (canCastle(colour, true)) {
        uint64_t path = indexToBit(rank + 5) | indexToBit(rank + 6);
        if (!(path & occupied) && 
            !isSquareAttacked(rank + 5, 1 - colour) && 
            !isSquareAttacked(rank + 6, 1 - colour)) {
            moves.add(setCastle(encodeMove(rank + 4, rank + 6), true));
        }
    }
    
    if (canCastle(colour, false)) {
        uint64_t path = indexToBit(rank + 1) | indexToBit(rank + 2) | indexToBit(rank + 3);
        if (!(path & occupied) && 
            !isSquareAttacked(rank + 2, 1 - colour) && 
            !isSquareAttacked(rank + 3, 1 - colour)) {
            moves.add(setCastle(encodeMove(rank + 4, rank + 2), true));
        }
    }
}

bool Board::canCastle(int colour, bool kingside) {
    if (colour == WHITE) {
        return kingside ? (castlingRights & WHITEKSIDE) : (castlingRights & WHITEQSIDE);
    } else {
        return kingside ? (castlingRights & BLACKKSIDE) : (castlingRights & BLACKQSIDE);
    }
}

void Board::getPseudoLegalQuiets(MoveList &quiets, int colour) {
    MoveList allMoves;
    getAllPseudoLegalMoves(allMoves, colour);
    
    quiets.clear();
    for (unsigned int i = 0; i < allMoves.size(); i++) {
        Move move = allMoves.get(i);
        if (!isCapture(move)) {
            quiets.add(move);
        }
    }
}

void Board::getPseudoLegalCaptures(MoveList &captures, int colour, bool includePromotions) {
    MoveList allMoves;
    getAllPseudoLegalMoves(allMoves, colour);
    
    captures.clear();
    for (unsigned int i = 0; i < allMoves.size(); i++) {
        Move move = allMoves.get(i);
        if (isCapture(move) || (includePromotions && isPromotion(move))) {
            captures.add(move);
        }
    }
}

MoveList Board::getAllLegalMove(int colour) {
    MoveList pseudoLegal;
    MoveList legal;
    
    getAllPseudoLegalMoves(pseudoLegal, colour);
    
    for (unsigned int i = 0; i < pseudoLegal.size(); i++) {
        Move move = pseudoLegal.get(i);
        if (isLegalMove(move, colour)) {
            legal.add(move);
        }
    }
    
    return legal;
}

bool Board::isLegalMove(Move move, int colour) {
    Board tempBoard = staticCopy();
    tempBoard.doMove(move, colour);
    return !tempBoard.isInCheck(colour);
}

uint64_t Board::getPinnedMap(int colour) {
    uint64_t pinned = 0;
    uint64_t kingBB = pieces[colour][KINGS];
    if (!kingBB) return 0;
    
    int kingSq = bitScanForward(kingBB);
    uint64_t occupied = allPieces[WHITE] | allPieces[BLACK];
    uint64_t enemies = allPieces[1 - colour];
    
    uint64_t rookAttackers = getRookAttacks(kingSq, enemies) & 
                            (pieces[1 - colour][ROOKS] | pieces[1 - colour][QUEENS]);
    
    while (rookAttackers) {
        int attackerSq = bitScanForward(rookAttackers);
        uint64_t between = inBetweenSqs[kingSq][attackerSq];
        uint64_t blockers = between & occupied;
        
        if (count(blockers) == 1 && (blockers & allPieces[colour])) {
            pinned |= blockers;
        }
        rookAttackers &= rookAttackers - 1;
    }
    
    uint64_t bishopAttackers = getBishopAttacks(kingSq, enemies) & 
                              (pieces[1 - colour][BISHOPS] | pieces[1 - colour][QUEENS]);
    
    while (bishopAttackers) {
        int attackerSq = bitScanForward(bishopAttackers);
        uint64_t between = inBetweenSqs[kingSq][attackerSq];
        uint64_t blockers = between & occupied;
        
        if (count(blockers) == 1 && (blockers & allPieces[colour])) {
            pinned |= blockers;
        }
        bishopAttackers &= bishopAttackers - 1;
    }
    
    return pinned;
}

bool Board::doPseudoLegalMove(Move m, int colour) {
    doMove(m, colour);
    return true;
}

bool Board::doHashMove(Move m, int colour) {
    return doPseudoLegalMove(m, colour);
}

void Board::doNullMove() {
    playerToMove = 1 - playerToMove;
    zobristKey ^= zobristSide;
    if (epCaptureFile != NO_EP_POSSIBLE) {
        zobristKey ^= zobristEP[epCaptureFile];
        epCaptureFile = NO_EP_POSSIBLE;
    }
}

void Board::undoNullMove(uint16_t _epCaptureFile) {
    playerToMove = 1 - playerToMove;
    zobristKey ^= zobristSide;
    epCaptureFile = _epCaptureFile;
    if (epCaptureFile != NO_EP_POSSIBLE) {
        zobristKey ^= zobristEP[epCaptureFile];
    }
}

PieceMoveList Board::getPieceMoveList(int colour) {
    PieceMoveList pml;
    MoveList moves;
    getAllPseudoLegalMoves(moves, colour);
    
    for (unsigned int i = 0; i < moves.size(); i++) {
        Move move = moves.get(i);
        int startSq = getStartSq(move);
        int pieceID = getPieceOnSquare(colour, startSq);
        uint64_t legal = indexToBit(getEndSq(move));
        pml.add(PieceMoveInfo(pieceID, startSq, legal));
    }
    
    return pml;
}

void Board::getPseudoLegalPromotions(MoveList &moves, int colour) {
    MoveList allMoves;
    getAllPseudoLegalMoves(allMoves, colour);
    
    moves.clear();
    for (unsigned int i = 0; i < allMoves.size(); i++) {
        Move move = allMoves.get(i);
        if (isPromotion(move)) {
            moves.add(move);
        }
    }
}

void Board::getPseudoLegalChecks(MoveList &checks, int colour) {
    MoveList allMoves;
    getAllPseudoLegalMoves(allMoves, colour);
    
    checks.clear();
    for (unsigned int i = 0; i < allMoves.size(); i++) {
        Move move = allMoves.get(i);
        Board tempBoard = staticCopy();
        tempBoard.doMove(move, colour);
        if (tempBoard.isInCheck(1 - colour)) {
            checks.add(move);
        }
    }
}

void Board::getPseudoLegalCheckEscapes(MoveList &escapes, int colour) {
    if (!isInCheck(colour)) {
        getAllPseudoLegalMoves(escapes, colour);
        return;
    }
    
    MoveList allMoves;
    getAllPseudoLegalMoves(allMoves, colour);
    
    escapes.clear();
    for (unsigned int i = 0; i < allMoves.size(); i++) {
        Move move = allMoves.get(i);
        if (isLegalMove(move, colour)) {
            escapes.add(move);
        }
    }
}

uint64_t Board::getXRayPieceMap(int, int sq, int, 
                               uint64_t blockerStart, uint64_t blockerEnd) {
    uint64_t occupied = allPieces[WHITE] | allPieces[BLACK];
    occupied ^= blockerStart;
    occupied |= blockerEnd;
    
    return getRookAttacks(sq, occupied) | getBishopAttacks(sq, occupied);
}

uint64_t Board::getAttackMap(int colour, int sq) {
    uint64_t occupied = allPieces[WHITE] | allPieces[BLACK];
    
    uint64_t attacks = 0;
    attacks |= getPawnAttacks(sq, 1 - colour) & pieces[colour][PAWNS];
    attacks |= KNIGHTMOVES[sq] & pieces[colour][KNIGHTS];
    attacks |= KINGMOVES[sq] & pieces[colour][KINGS];
    attacks |= getRookAttacks(sq, occupied) & (pieces[colour][ROOKS] | pieces[colour][QUEENS]);
    attacks |= getBishopAttacks(sq, occupied) & (pieces[colour][BISHOPS] | pieces[colour][QUEENS]);
    
    return attacks;
}

uint64_t Board::getAttackMap(int sq) {
    return getAttackMap(WHITE, sq) | getAttackMap(BLACK, sq);
}

int Board::getPieceOnSquare(int colour, int sq) {
    uint64_t sqBit = indexToBit(sq);
    for (int piece = 0; piece < 6; piece++) {
        if (pieces[colour][piece] & sqBit) {
            return piece;
        }
    }
    return -1;
}

bool Board::isCheckMove(int colour, int sq) {
    Board tempBoard = staticCopy();
    return tempBoard.isSquareAttacked(sq, 1 - colour);
}

uint64_t Board::getRookXRays(int sq, uint64_t occ, uint64_t blockers) {
    return getRookAttacks(sq, occ ^ blockers);
}

uint64_t Board::getBishopXRays(int sq, uint64_t occ, uint64_t blockers) {
    return getBishopAttacks(sq, occ ^ blockers);
}

bool Board::isDraw() {
    if (fiftyMoveCounter >= 100) return true;
    
    if (isInsufficientMaterial()) return true;
    
    if (!isInCheck(playerToMove)) {
        MoveList legalMoves = getAllLegalMove(playerToMove);
        if (legalMoves.size() == 0) return true;
    }
    
    return false;
}

bool Board::isInsufficientMaterial() {
    if (count(allPieces[WHITE]) == 1 && count(allPieces[BLACK]) == 1) return true;
    
    for (int color = 0; color < 2; color++) {
        int opponent = 1 - color;
        if (count(allPieces[color]) == 1 && count(allPieces[opponent]) == 2) {
            if (pieces[opponent][KNIGHTS] || pieces[opponent][BISHOPS]) {
                return true;
            }
        }
    }
    
    if (count(allPieces[WHITE]) == 2 && count(allPieces[BLACK]) == 2 &&
        pieces[WHITE][BISHOPS] && pieces[BLACK][BISHOPS]) {
        
        bool whiteLightSquare = (pieces[WHITE][BISHOPS] & LIGHT) != 0;
        bool blackLightSquare = (pieces[BLACK][BISHOPS] & LIGHT) != 0;
        
        if (whiteLightSquare == blackLightSquare) return true;
    }
    
    return false;
}

void Board::getCheckMaps(int colour, uint64_t *checkMaps) {
    uint64_t kingBB = pieces[colour][KINGS];
    if (!kingBB) return;
    
    int kingSq = bitScanForward(kingBB);
    uint64_t occupied = allPieces[WHITE] | allPieces[BLACK];
    
    checkMaps[PAWNS] = getPawnAttacks(kingSq, colour);
    checkMaps[KNIGHTS] = KNIGHTMOVES[kingSq];
    checkMaps[BISHOPS] = getBishopAttacks(kingSq, occupied);
    checkMaps[ROOKS] = getRookAttacks(kingSq, occupied);
    checkMaps[QUEENS] = checkMaps[BISHOPS] | checkMaps[ROOKS];
    checkMaps[KINGS] = 0;
}

int Board::getMaterial(int colour) {
    return count(pieces[colour][PAWNS]) * 100 +
           count(pieces[colour][KNIGHTS]) * 320 +
           count(pieces[colour][BISHOPS]) * 330 +
           count(pieces[colour][ROOKS]) * 500 +
           count(pieces[colour][QUEENS]) * 900;
}