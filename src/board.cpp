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
    // Initialize starting position
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            pieces[i][j] = 0;
        }
        allPieces[i] = 0;
    }
    
    // White pieces
    pieces[WHITE][PAWNS] = 0x000000000000FF00ULL;
    pieces[WHITE][ROOKS] = 0x0000000000000081ULL;
    pieces[WHITE][KNIGHTS] = 0x0000000000000042ULL;
    pieces[WHITE][BISHOPS] = 0x0000000000000024ULL;
    pieces[WHITE][QUEENS] = 0x0000000000000008ULL;
    pieces[WHITE][KINGS] = 0x0000000000000010ULL;
    
    // Black pieces
    pieces[BLACK][PAWNS] = 0x00FF000000000000ULL;
    pieces[BLACK][ROOKS] = 0x8100000000000000ULL;
    pieces[BLACK][KNIGHTS] = 0x4200000000000000ULL;
    pieces[BLACK][BISHOPS] = 0x2400000000000000ULL;
    pieces[BLACK][QUEENS] = 0x0800000000000000ULL;
    pieces[BLACK][KINGS] = 0x1000000000000000ULL;
    
    // Update combined bitboards
    for (int i = 0; i < 2; i++) {
        allPieces[i] = 0;
        for (int j = 0; j < 6; j++) {
            allPieces[i] |= pieces[i][j];
        }
    }
    
    // Game state
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
    
    // Clear bitboards
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            pieces[i][j] = 0;
        }
        allPieces[i] = 0;
    }
    
    // Convert mailbox to bitboards
    for (int sq = 0; sq < 64; sq++) {
        int piece = mailboxBoard[sq];
        if (piece != -1) {
            int color = piece / 6;
            int pieceType = piece % 6;
            pieces[color][pieceType] |= indexToBit(sq);
            allPieces[color] |= indexToBit(sq);
        }
    }
    
    // Set game state
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
    
    // Hash pieces
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
    
    // Hash castling rights
    key ^= zobristCastling[castlingRights];
    
    // Hash en passant
    if (epCaptureFile != NO_EP_POSSIBLE) {
        key ^= zobristEP[epCaptureFile];
    }
    
    // Hash side to move
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
    
    // Find piece type
    int pieceType = -1;
    for (int p = 0; p < 6; p++) {
        if (pieces[colour][p] & startBit) {
            pieceType = p;
            break;
        }
    }
    
    // Move piece
    pieces[colour][pieceType] &= ~startBit;
    pieces[colour][pieceType] |= endBit;
    allPieces[colour] &= ~startBit;
    allPieces[colour] |= endBit;
    
    // Handle captures
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
    
    // Handle special moves
    if (isCastle(m)) {
        // Handle castling rook movement
        if (endSq == 6) { // White kingside
            pieces[WHITE][ROOKS] &= ~indexToBit(7);
            pieces[WHITE][ROOKS] |= indexToBit(5);
            allPieces[WHITE] &= ~indexToBit(7);
            allPieces[WHITE] |= indexToBit(5);
        } else if (endSq == 2) { // White queenside
            pieces[WHITE][ROOKS] &= ~indexToBit(0);
            pieces[WHITE][ROOKS] |= indexToBit(3);
            allPieces[WHITE] &= ~indexToBit(0);
            allPieces[WHITE] |= indexToBit(3);
        } else if (endSq == 62) { // Black kingside
            pieces[BLACK][ROOKS] &= ~indexToBit(63);
            pieces[BLACK][ROOKS] |= indexToBit(61);
            allPieces[BLACK] &= ~indexToBit(63);
            allPieces[BLACK] |= indexToBit(61);
        } else if (endSq == 58) { // Black queenside
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
    
    // Update castling rights
    if (pieceType == KINGS) {
        castlingRights &= (colour == WHITE) ? ~WHITECASTLE : ~BLACKCASTLE;
    }
    if (pieceType == ROOKS) {
        if (startSq == 0) castlingRights &= ~WHITEQSIDE;
        if (startSq == 7) castlingRights &= ~WHITEKSIDE;
        if (startSq == 56) castlingRights &= ~BLACKQSIDE;
        if (startSq == 63) castlingRights &= ~BLACKKSIDE;
    }
    
    // Update en passant
    epCaptureFile = NO_EP_POSSIBLE;
    if (pieceType == PAWNS && abs(endSq - startSq) == 16) {
        epCaptureFile = endSq & 7;
    }
    
    // Update counters
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
    
    // Check pawn attacks
    uint64_t pawnAttacks = getPawnAttacks(sq, 1 - byColour);
    if (pawnAttacks & pieces[byColour][PAWNS]) return true;
    
    // Check knight attacks
    if (KNIGHTMOVES[sq] & pieces[byColour][KNIGHTS]) return true;
    
    // Check king attacks
    if (KINGMOVES[sq] & pieces[byColour][KINGS]) return true;
    
    // Check sliding piece attacks
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

int Board::getMaterial(int colour) {
    return count(pieces[colour][PAWNS]) * 100 +
           count(pieces[colour][KNIGHTS]) * 320 +
           count(pieces[colour][BISHOPS]) * 330 +
           count(pieces[colour][ROOKS]) * 500 +
           count(pieces[colour][QUEENS]) * 900;
}