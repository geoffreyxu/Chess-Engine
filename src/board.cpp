/**
 * Constructs a new Board object to the starting chess position.
 */

#include "board.hpp"
#include <random>
using namespace std;

// Holds zobrist random values
namespace Zobrist {
    unsigned long long pieces[12][64];
    unsigned long long blackMove;
    unsigned long long castling[4];
    unsigned long long enPassant[8];
};


// Initializes 81 random 64-bit numbers.
void initZobrist() {
    random_device rd;
    mt19937_64 eng(rd());
    uniform_int_distribution<unsigned long long> distr;
    for (int color = nWhite; color <= nBlack; color++) {
        for (int piece = nPawn; piece <= nKing; piece++) {
            for (int sq = A1; sq <= H8; sq++) {
                Zobrist::pieces[2 * color + piece][sq] = distr(eng);
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        Zobrist::castling[i] = distr(eng);
    }
    for (int i = 0; i < 8; i++) {
        Zobrist::enPassant[i] = distr(eng);
    }
    Zobrist::blackMove = distr(eng);
}


// Initializes the zobrist hash key to the current board position
void Board::setZobrist() {
    unsigned long long hashKey = 0;
    for (int color = nWhite; color <= nBlack; color++) {
        for (int piece = nPawn; piece <= nKing; piece++) {
            Bitboard pieces = getPieces((Color)color, (Piece)piece);
            while (pieces) {
                int sq = pop_lsb(&pieces);
                hashKey ^= Zobrist::pieces[2 * color + piece][sq];
            }
        }
    }

    short castle = castling.top();
    for (int i = 0; i < 4; i++) {
        if (castle & (1 << i)) {
            hashKey ^= Zobrist::castling[i];
        }
    }

    if (toMove == nBlack) {
        hashKey ^= Zobrist::blackMove;
    }

    if (enPassant.top() != SQ_NONE) {
        hashKey ^= Zobrist::enPassant[enPassant.top() % 8];
    }
    zobrist.push_back(hashKey);
}


// Constructs a new Board object
Board::Board() {
    initZobrist();

    // initialize pieces
    pieceBB[0] = Rank1 | Rank2;
    pieceBB[1] = Rank7 | Rank8;
    pieceBB[2] = Rank2 | Rank7;
    pieceBB[3] = sqToBB[B1] | sqToBB[G1] | sqToBB[B8] | sqToBB[G8];
    pieceBB[4] = sqToBB[C1] | sqToBB[F1] | sqToBB[C8] | sqToBB[F8];
    pieceBB[5] = sqToBB[A1] | sqToBB[H1] | sqToBB[A8] | sqToBB[H8];
    pieceBB[6] = sqToBB[D1] | sqToBB[D8];
    pieceBB[7] = sqToBB[E1] | sqToBB[E8];

    occupiedBB = (pieceBB[0] | pieceBB[1]);
    emptyBB = ~occupiedBB;

    enPassant.push(SQ_NONE);
    castling.push(0b1111);
    capturedList.push(PIECE_NONE);
    toMove = nWhite;
    fiftyList.push(0);
    fullMove = 1;

    setZobrist();
}


/**
 * Constructs a new Board object to the given position
 *
 * @param FEN the desired starting position in FEN form
 */
Board::Board(std::string FEN) {
    initZobrist();
    setPosition(FEN);
}


/**
 * Given the current board, sets the position of the board to the given FEN.
 *
 * @param FEN the desired position the board will be sent to.
 */
void Board::setPosition(std::string FEN) {
    enPassant = stack<Square>();
    castling = stack<short>();
    fiftyList = stack<int>();
    capturedList = stack<Piece>();
    zobrist = vector<unsigned long long>();
    //for (int i = 0; i < 100000; i++)
    //    transTable[i] = HashEntry();
    // initializes killer move list
    //for (int i = 0; i < SEARCH_DEPTH + 1; i++)
    //    for (int j = 0; j < 2; j++)
    //        killerMoves[i][j] = Move();

    pieceBB[0] = 0; 
    pieceBB[1] = 0;
    pieceBB[2] = 0;
    pieceBB[3] = 0;
    pieceBB[4] = 0;
    pieceBB[5] = 0;
    pieceBB[6] = 0;
    pieceBB[7] = 0;

    std::vector<std::string> result;
    std::vector<std::string> pieceList;
    std::istringstream iss(FEN);
    for(std::string s; iss >> s; )
        result.push_back(s);    

    assert(result.size() == 6 || result.size() == 4);
    std::string pieces = result[0];
    std::replace(pieces.begin(), pieces.end(), '/', ' ');

    std::istringstream pieceISS(pieces);
    for(std::string s; pieceISS >> s; )
        pieceList.push_back(s);    

    // Reads in piece info from the FEN
    assert(pieceList.size() == 8);
    for (int i = 0; i < 8; i++) {
        std::string row = pieceList[i];
        int currSq = 0;
        for (char& ch : row) {
            if (isdigit(ch)) {
                currSq += ch - '0';
                continue;
            }
            Color c = isupper(ch) ? nWhite : nBlack;
            ch = tolower(ch);
            Piece p;
            switch (ch) {
                case 'p':
                    p = nPawn;
                    break;
                case 'n':
                    p = nKnight;
                    break;
                case 'b':
                    p = nBishop;
                    break;
                case 'r':
                    p = nRook;
                    break;
                case 'q':
                    p = nQueen;
                    break;
                case 'k':
                    p = nKing;
                    break;
                default:
                    assert(false);
            }
            pieceBB[c] |= sqToBB[(7 - i) * 8 + currSq];
            pieceBB[2 + p] |= sqToBB[(7 - i) * 8 + currSq];
            currSq++;
        }
    }

    toMove = (result[1] == "w" ? nWhite : nBlack);

    // castling rights
    short castle = 0;
    for (char& ch : result[2]) {
        switch(ch) {
            case 'K':
                castle |= 0b1000;
                break;
            case 'Q':
                castle |= 0b0100;
                break;
            case 'k':
                castle |= 0b0010;
                break;
            case 'q':
                castle |= 0b0001;
                break;
        }
    }
    castling.push(castle);

    // en passant
    if (result[3] == "-") {
        enPassant.push(SQ_NONE);
    } else {
        enPassant.push((Square)(std::find(squareNames, squareNames+65, result[3]) -
                    squareNames));
    }

    if (result.size() == 6) {
        fiftyList.push(stoi(result[4]));
        fullMove = stoi(result[5]);
    } else {
        fiftyList.push(0);
        fullMove = 1;
    }

    occupiedBB = (pieceBB[0] | pieceBB[1]);
    emptyBB = ~occupiedBB;

    setZobrist(); 
}


// Returns the current Board's FEN state.
std::string Board::getFEN() const {
    std::string FEN = "";
    for (int row = 7; row >= 0; row--) {
        int emptyCount = 0;
        for (int col = 0; col < 8; col++) {
            int sq = 8 * row + col;
            if (getPiece(sq) == PIECE_NONE) {
                emptyCount++;
                continue;
            }
            if (emptyCount != 0) {
                FEN += std::to_string(emptyCount);
                emptyCount = 0;
            }
            char p;
            switch(getPiece(sq)) {
                case nPawn:
                    p = 'p';
                    break;
                case nKnight:
                    p = 'n';
                    break;
                case nBishop:
                    p = 'b';
                    break;
                case nRook:
                    p = 'r';
                    break;
                case nQueen:
                    p = 'q';
                    break;
                case nKing:
                    p = 'k';
                    break;
                default:
                    break;
            }
            if (getColor(sq) == nWhite) {
                p = toupper(p); 
            }
            FEN += p;
        }
        if (emptyCount != 0) {
            FEN += std::to_string(emptyCount);
        }
        if (row != 0) {
            FEN += "/";
        }
    }
    FEN += " ";
    FEN += (toMove == nWhite ? "w" : "b");
    FEN += " ";

    short castle = castling.top();
    if (castle & 0b1000) {
        FEN += "K";
    }  
    if (castle & 0b0100) {
        FEN += "Q";
    } 
    if (castle & 0b0010) {
        FEN += "k";
    } 
    if (castle & 0b0001) {
        FEN += "q";
    }
    if (castle) {
        FEN += " ";
    } else {
        FEN += "- ";
    }

    FEN += (enPassant.top() == SQ_NONE ? "-" : squareNames[enPassant.top()]);
    FEN += " ";

    FEN += std::to_string(fiftyList.top()) + " " + std::to_string(fullMove);

    return FEN;
}


// Returns pieces of the given piece type
Bitboard Board::getPieces(Piece pt) const {
    return pieceBB[2 + pt];
}


// Returns pieces of the given color
Bitboard Board::getPieces(Color ct) const {
    return pieceBB[ct];
}


// Returns pieces of the given piece and color
Bitboard Board::getPieces(Color ct, Piece pt) const {
    return pieceBB[2 + pt] & pieceBB[ct];
}


// Returns the piece on a given square
Piece Board::getPiece(int sq) const {
    Bitboard sqBB = sqToBB[sq];
    if (sqBB & pieceBB[2 + nPawn]) {
        return nPawn;
    } else if (sqBB & pieceBB[2 + nKnight]) {
        return nKnight;
    } else if (sqBB & pieceBB[2 + nBishop]) {
        return nBishop;
    } else if (sqBB & pieceBB[2 + nRook]) {
        return nRook;
    } else if (sqBB & pieceBB[2 + nQueen]) {
        return nQueen;
    } else if (sqBB & pieceBB[2 + nKing]) {
        return nKing;
    } else {
        return PIECE_NONE;
    }
}


// Returns the color of the piece on the given square
Color Board::getColor(int sq) const {
    Bitboard sqBB = sqToBB[sq];
    if (sqBB & pieceBB[nWhite]) {
        return nWhite;
    } else if (sqBB & pieceBB[nBlack]) {
        return nBlack;
    } else {
        return COLOR_NONE;
    }
}


// Returns a bitboard representing the file the current square is on.
Bitboard Board::getFile(Square sq) const {
    return (0x0101010101010101 << (sq & 7));
}


// Returns a bitboard representing the rank the current square is on.
Bitboard Board::getRank(Square sq) const {
    return (0x11111111 << (sq / 7));
}

// Returns a bitboard holding the locations of the white pawns
Bitboard Board::getWhitePawns() const {
    return pieceBB[nWhite] & pieceBB[2];
}


// Returns a bitboard holding the locations of the black pawns
Bitboard Board::getBlackPawns() const {
    return pieceBB[nBlack] & pieceBB[2];
}


// Returns a bitboard holding locations of all occupied squares
Bitboard Board::getOccupied() const {
    return occupiedBB;
}


// Returns a bitboard holding locations of all empty squares
Bitboard Board::getEmpty() const {
    return emptyBB;
}


// Returns the square that is the en passant target, if it exists
Square Board::enPassantTarget() const {
    return enPassant.top();
}


// Returns the castling rights of the board
short Board::getCastlingRights() const {
    return castling.top();
}


// Returns the current side to move
Color Board::getToMove() const {
    return toMove;
}


// Returns the zobrist hash key
unsigned long long Board::getZobrist() const {
    return zobrist.back();
}


// Returns the fifty move counter
int Board::getFiftyCount() const {
    return fiftyList.top();
}


// Returns whether a square is attacked by a given side
bool Board::attacked(int square, Color side) const {
    Bitboard pawns = getPieces(side, nPawn);
    if (pawnAttacks[side^1][square] & pawns) {
        return true;
    }
    Bitboard knights = getPieces(side, nKnight);
    if (knightAttacks[square] & knights) {
        return true;
    }
    Bitboard king = getPieces(side, nKing);
    if (kingAttacks[square] & king) {
        return true;
    }
    Bitboard bishopsQueens = getPieces(side, nQueen) | getPieces(side, nBishop);
    if (slidingAttacksBB<nBishop>(square, occupiedBB) & bishopsQueens) {
        return true;
    }
    Bitboard rooksQueens = getPieces(side, nQueen) | getPieces(side, nRook);
    if (slidingAttacksBB<nRook>(square, occupiedBB) & rooksQueens) {
        return true;
    }
    return false;
}


// Returns a bitboard holding the pieces attacking the square of the given
// color
Bitboard Board::getAttackers(Square sq, Color c) const {
    Color other = (toMove == nWhite ? nBlack : nWhite);
    Bitboard attackers = 0;

    attackers |= (pawnAttacks[toMove][sq] & getPieces(other, nPawn));
    attackers |= (knightAttacks[sq] & getPieces(other, nKnight));

    Bitboard bishopsQueens = getPieces(other, nQueen) | getPieces(other,
            nBishop);
    attackers |= (slidingAttacksBB<nBishop>(sq, occupiedBB) & bishopsQueens);

    Bitboard rooksQueens = getPieces(other, nQueen) | getPieces(other, nRook);
    attackers |= (slidingAttacksBB<nRook>(sq, occupiedBB) & rooksQueens);

    Bitboard kings = getPieces(other, nKing);
    attackers |= (kingAttacks[sq] & kings);

    return attackers;
}


// Returns a color's least valuable attacker of a square
Square Board::lva(Square sq, Color side) const {
    Bitboard pawns = getPieces(side, nPawn);
    if (pawnAttacks[side^1][sq] & pawns) {
        return lsb(pawnAttacks[side^1][sq] & pawns);
    }

    Bitboard knights = getPieces(side, nKnight);
    if (knightAttacks[sq] & knights) {
        return lsb(knightAttacks[sq] & knights);
    }

    Bitboard bishops = getPieces(side, nBishop);
    Bitboard bishopAttackers = slidingAttacksBB<nBishop>(sq, occupiedBB) & bishops;
    if (bishopAttackers) {
        return lsb(bishopAttackers);
    }

    Bitboard rooks = getPieces(side, nRook);
    Bitboard rookAttackers = slidingAttacksBB<nRook>(sq, occupiedBB) & rooks;
    if (rookAttackers) {
        return lsb(rookAttackers);
    }

    Bitboard queens = getPieces(side, nRook);
    Bitboard queenAttackers = (slidingAttacksBB<nBishop>(sq, occupiedBB) |
            slidingAttacksBB<nRook>(sq, occupiedBB)) & queens;
    if (queenAttackers) {
        return lsb(queenAttackers);
    }

    Bitboard king = getPieces(side, nKing);
    if (kingAttacks[sq] & king) {
        return lsb(king);
    }
    return SQ_NONE;
}


// Returns whether a board is in check or not
bool Board::inCheck() const {
    Square kingSquare = lsb(getPieces(toMove, nKing)); 
    Color other = (toMove == nWhite ? nBlack : nWhite);
    return(attacked(kingSquare, other));
}


// Returns whether a board is in double check or not
bool Board::doubleCheck() const {
    int kingSquare = lsb(getPieces(toMove, nKing)); 
    Color other = (toMove == nWhite ? nBlack : nWhite);
    int count = 0;

    Bitboard knights = getPieces(other, nKnight);
    if (knightAttacks[kingSquare] & knights) {
        count++;
    }
    Bitboard bishopsQueens = getPieces(other, nQueen) | getPieces(other,
            nBishop);
    if (slidingAttacksBB<nBishop>(kingSquare, occupiedBB) & bishopsQueens) {
        if (count == 1) {
            return true;
        }
        count++;
    }

    if (count == 0) {
        return false;
    }

    Bitboard rooksQueens = getPieces(other, nQueen) | getPieces(other, nRook);
    if (slidingAttacksBB<nRook>(kingSquare, occupiedBB) & rooksQueens) {
        return true;
    }
    return false;
}


// Returns a bitboard holding the pieces checking the king
Bitboard Board::getCheckers() const {
    int kingSquare = lsb(getPieces(toMove, nKing)); 
    Color other = (toMove == nWhite ? nBlack : nWhite);
    Bitboard checkers = 0;

    checkers |= (pawnAttacks[toMove][kingSquare] & getPieces(other, nPawn));
    checkers |= (knightAttacks[kingSquare] & getPieces(other, nKnight));

    Bitboard bishopsQueens = getPieces(other, nQueen) | getPieces(other,
            nBishop);
    checkers |= (slidingAttacksBB<nBishop>(kingSquare, occupiedBB) & bishopsQueens);

    Bitboard rooksQueens = getPieces(other, nQueen) | getPieces(other, nRook);
    checkers |= (slidingAttacksBB<nRook>(kingSquare, occupiedBB) & rooksQueens);

    return checkers;
}


// returns all pieces that are pinned to a square by a piece of either color
Bitboard Board::pinnedPieces(Bitboard pinners, Square sq) const {
    Bitboard pinned = 0;
    // get the list of possible attackers/pinners
    Bitboard attackers = ((slidingAttacksBB<nBishop>(sq, 0) & (getPieces(nBishop) |
        getPieces(nQueen))) | (slidingAttacksBB<nRook>(sq, 0) &
        (getPieces(nRook) | getPieces(nQueen)))) & pinners;

    while (attackers) {
        int attackSq = pop_lsb(&attackers);
        Bitboard blockers = betweenBB[attackSq][sq] & occupiedBB;
        if (popcount(blockers) == 1) {
            pinned |= blockers; 
        }
    }

    return pinned;
}


// Makes a legal move on the chessboard
void Board::makeMove(Move m) {
    unsigned long long hashKey = zobrist.back();
    // increments move counters
    int fiftyCounter = fiftyList.top() + 1;
    if (toMove == nBlack) { 
        fullMove++;
    }

    hashKey ^= Zobrist::blackMove;

    toMove = (toMove == nWhite ? nBlack : nWhite);

    // Get all the information from the move.
    int start = m.getFrom();
    int end = m.getTo();
    int flags = m.getFlags();
    bool capture = m.isCapture();
    bool prom = m.isPromotion();
    // Bitboards corresponding to start and end square.
    Bitboard startBB = sqToBB[start];
    Bitboard endBB = sqToBB[end];
    Bitboard startEndBB = startBB | endBB;
    // Get the start and end pieces.
    Piece startP = getPiece(start);
    Color startC = getColor(start);
    Piece endP = getPiece(end);
    Color endC = getColor(end);

    pieceBB[(int)startP + 2] ^= startEndBB; 
    pieceBB[(int)startC] ^= startEndBB;

    hashKey ^= Zobrist::pieces[2 * startC + startP][start];
    hashKey ^= Zobrist::pieces[2 * startC + startP][end];

    short newCastling = castling.top();

    // Resets fifty move counter if pawn move or capture
    if (startP == nPawn || capture) {
        fiftyCounter = 0;
    }

    if (enPassant.top() != SQ_NONE) {
        hashKey ^= Zobrist::enPassant[enPassant.top() % 8];
    }

    // Double pawn move
    if (flags == 1) {
        enPassant.push((Square)(startC == nWhite ? end - 8 : end + 8));
        hashKey ^= Zobrist::enPassant[enPassant.top() % 8];
    } else {
        enPassant.push(SQ_NONE);
    }

    if (flags == 5) { // en passant
        endP = nPawn;
        if (startC == nWhite) {
            pieceBB[2] ^= sqToBB[end - 8];    
            pieceBB[1] ^= sqToBB[end - 8];
            hashKey ^= Zobrist::pieces[2 * nBlack + nPawn][end - 8];
        } else {
            pieceBB[2] ^= sqToBB[end + 8];    
            pieceBB[0] ^= sqToBB[end + 8];
            hashKey ^= Zobrist::pieces[2 * nWhite + nPawn][end + 8];
        }
    } else if (capture) {
        pieceBB[(int) endP + 2] ^= endBB;
        pieceBB[(int) endC] ^= endBB;
        hashKey ^= Zobrist::pieces[2 * endC + endP][end];
    }

    if (prom) {
        int promPiece = 1 + (flags & 3);
        pieceBB[promPiece + 2] ^= endBB;
        pieceBB[2] ^= endBB;
        hashKey ^= Zobrist::pieces[2 * startC + promPiece][end];
        hashKey ^= Zobrist::pieces[2 * startC + nPawn][end];
    } 

    if (flags == 2) { // castling
        if (startC == nWhite) {
            pieceBB[nRook + 2] ^= (sqToBB[F1] | sqToBB[H1]);
            pieceBB[startC] ^= (sqToBB[F1] | sqToBB[H1]);
            hashKey ^= Zobrist::pieces[2 * nWhite + nRook][F1];
            hashKey ^= Zobrist::pieces[2 * nWhite + nRook][H1];
            newCastling &= 0b0011;
        } else { 
            pieceBB[nRook + 2] ^= (sqToBB[F8] | sqToBB[H8]);
            pieceBB[startC] ^= (sqToBB[F8] | sqToBB[H8]);
            hashKey ^= Zobrist::pieces[2 * nBlack + nRook][F8];
            hashKey ^= Zobrist::pieces[2 * nBlack + nRook][H8];
            newCastling &= 0b1100;
        }
    } else if (flags == 3)  { // queenside
        if (startC == nWhite) {
            pieceBB[nRook + 2] ^= (sqToBB[A1] | sqToBB[D1]);
            pieceBB[startC] ^= (sqToBB[A1] | sqToBB[D1]);
            hashKey ^= Zobrist::pieces[2 * nWhite + nRook][A1];
            hashKey ^= Zobrist::pieces[2 * nWhite + nRook][D1];
            newCastling &= 0b0011;
        } else { 
            pieceBB[nRook + 2] ^= (sqToBB[A8] | sqToBB[D8]);
            pieceBB[startC] ^= (sqToBB[A8] | sqToBB[D8]);
            hashKey ^= Zobrist::pieces[2 * nBlack + nRook][A8];
            hashKey ^= Zobrist::pieces[2 * nBlack + nRook][D8];
            newCastling &= 0b1100;
        }
    } 

    if (startP == nRook) {
        if (start == A1) {
            newCastling &= 0b1011;
        } else if (start == H1) {
            newCastling &= 0b0111;
        } else if (start == A8) {
            newCastling &= 0b1110;
        } else if (start == H8) {
            newCastling &= 0b1101;
        }
    }

    if (endP == nRook) {
        if (end == A1) {
            newCastling &= 0b1011;
        } else if (end == H1) {
            newCastling &= 0b0111;
        } else if (end == A8) {
            newCastling &= 0b1110;
        } else if (end == H8) {
            newCastling &= 0b1101;
        }
    }

    if (startP == nKing) {
        if (startC == nWhite) {
            newCastling &= 0b0011;
        } else {
            newCastling &= 0b1100;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (newCastling & (1 << i)) {
            hashKey ^= Zobrist::castling[i];
        }
    }

    castling.push(newCastling);

    occupiedBB = (pieceBB[0] | pieceBB[1]);
    emptyBB = ~occupiedBB;
    capturedList.push(endP);
    fiftyList.push(fiftyCounter);
    zobrist.push_back(hashKey);
}


// Undoes the last move
void Board::unmakeMove(Move m) {
    // decrements full move counter
    if (toMove == nWhite) {
        fullMove--;
    }

    toMove = (toMove == nWhite ? nBlack : nWhite);
        
    Piece endP = capturedList.top();

    bool prom = m.isPromotion();
    bool capture = m.isCapture();

    // makes lists previous state
    castling.pop();
    enPassant.pop();
    capturedList.pop();
    fiftyList.pop();
    zobrist.pop_back();
    int start = m.getFrom();
    int end = m.getTo();
    int flags = m.getFlags();

    Color startC = getColor(end);
    Piece startP = (prom ? nPawn : getPiece(end));
    Color other = (startC == nWhite ? nBlack : nWhite);

    Bitboard startBB = sqToBB[start];
    Bitboard endBB = sqToBB[end];
    Bitboard startEndBB = startBB ^ endBB;

    pieceBB[(int)startP + 2] ^= startEndBB; 
    pieceBB[(int)startC] ^= startEndBB;

    if (flags == 5) { // en passant
        endP = nPawn;
        if (startC == nWhite) {
            pieceBB[2] ^= sqToBB[end - 8];    
            pieceBB[1] ^= sqToBB[end - 8];
        } else {
            pieceBB[2] ^= sqToBB[end + 8];    
            pieceBB[0] ^= sqToBB[end + 8];
        }
    } else if (capture) {
        pieceBB[(int) endP + 2] ^= endBB;
        pieceBB[(int) other] ^= endBB;
    }

    if (prom) {
        int promPiece = 1 + (flags & 3);
        pieceBB[promPiece + 2] ^= endBB;
        pieceBB[2] ^= endBB;
    } 

    if (flags == 2) { // kingside
        if (startC == nWhite) {
            pieceBB[nRook + 2] ^= (sqToBB[F1] | sqToBB[H1]);
            pieceBB[startC] ^= (sqToBB[F1] | sqToBB[H1]);
        } else { 
            pieceBB[nRook + 2] ^= (sqToBB[F8] | sqToBB[H8]);
            pieceBB[startC] ^= (sqToBB[F8] | sqToBB[H8]);
        }
    } else if (flags == 3) { // queenside
        if (startC == nWhite) {
            pieceBB[nRook + 2] ^= (sqToBB[A1] | sqToBB[D1]);
            pieceBB[startC] ^= (sqToBB[A1] | sqToBB[D1]);
        } else { 
            pieceBB[nRook + 2] ^= (sqToBB[A8] | sqToBB[D8]);
            pieceBB[startC] ^= (sqToBB[A8] | sqToBB[D8]);
        }
    } 

    occupiedBB = (pieceBB[0] | pieceBB[1]);
    emptyBB = ~occupiedBB;
}


// Makes a null move (switches color) for the current position
void Board::makeNullMove() {
    unsigned long long hashKey = zobrist.back();
    hashKey ^= Zobrist::blackMove;
    
    if (enPassant.top() != SQ_NONE) {
        hashKey ^= Zobrist::enPassant[enPassant.top() % 8];
    }
    enPassant.push(SQ_NONE);
    capturedList.push(PIECE_NONE);
    zobrist.push_back(hashKey);

    toMove = (toMove == nWhite ? nBlack : nWhite);
}


// Unmakes a null move for the current position
void Board::unmakeNullMove() {
    enPassant.pop();
    capturedList.pop();
    zobrist.pop_back();

    toMove = (toMove == nWhite ? nBlack : nWhite);
}


// Checks if a pseudo-legal move is legal
bool Board::isLegal(Move m) const {
    int start = m.getFrom();
    int end = m.getTo();
    Color startColor = getColor((Square)start);
    Color endColor = getColor((Square)end);
    Piece startPiece = getPiece((Square)start);
    Color other = (startColor == nWhite ? nBlack : nWhite);

    int pawnSq = end + 8 * (2 * startColor - 1);

    if (m.getFlags() == 1 && (abs(start - end) != 16 || startPiece != nPawn)) {
        return false;
    }

    if (toMove != startColor || toMove == endColor) {
        return false;
    }

    if (m.isCapture() && endColor == startColor) {
        return false;
    }
    
    if (m.isCapture() && (endColor == COLOR_NONE) && (m.getFlags() != 5)) {
        return false;
    }

    if (!m.isCapture() && endColor != COLOR_NONE) {
        return false;
    }

    if (sqToBB[end] & getPieces(other, nKing)) { // taking king
        return false;
    }
    if (m.getFlags() == 5) { // en passant
        if (end != enPassant.top()) {
            return false;
        }
        if (endColor != COLOR_NONE || startPiece != nPawn) {
            return false;
        }
        // check if there actually is an opposing pawn in the right place
        if (getPiece((Square)(pawnSq)) != nPawn) {
            return false;
        }

        Bitboard bishopsQueens = (pieceBB[nBishop + 2] | pieceBB[nQueen + 2]) &
            pieceBB[other];
        Bitboard rooksQueens = (pieceBB[nRook + 2] | pieceBB[nQueen + 2]) &
            pieceBB[other];
        Bitboard kingLoc = pieceBB[nKing + 2] & pieceBB[startColor];
        Bitboard newOccupied = occupiedBB ^ sqToBB[start] ^ sqToBB[end] ^ sqToBB[pawnSq];
        return !(allSlidingAttacks<nBishop>(bishopsQueens, newOccupied) & kingLoc) &&
                !(allSlidingAttacks<nRook>(rooksQueens, newOccupied) & kingLoc);
    }


    if (startPiece == nKing) {
        return m.getFlags() == 2 || m.getFlags() == 3 || !attacked((Square)m.getTo(),
                other);
    }

    int kingSquare = lsb(getPieces(toMove, nKing));
    if (!(pinnedPieces((getPieces(nBishop) | getPieces(nRook) | getPieces(nQueen)) &
            getPieces(other), (Square)kingSquare) & sqToBB[start])) {
        return true;
    }

    return lineBB[kingSquare][start] & sqToBB[end];
}


// Gets an entry from the transposition table
HashEntry Board::getTransTable(int key) const {
    return transTable[key];
}


// Updates an entry in the transposition table
void Board::setTransTable(int key, HashEntry entry) {
   transTable[key] = entry;
}


// Returns whether this position has been repeated at some point
bool Board::isRep() {
    unsigned long long z = zobrist.back();
    zobrist.pop_back();
    bool rep = find(zobrist.begin(), zobrist.end(), z) != zobrist.end();
    zobrist.push_back(z);
    return rep;
}


// Prints out the principal variation up to a given depth
void Board::printPV(int depth) {
    HashEntry tt = getTransTable(getZobrist() % 100000);
    Move m = tt.move;
    if (tt.nodeType != HASH_NULL && !(m == Move()) && depth != 0) {
        if (isLegal(m)) {
            makeMove(m);
            cout << " " << tt.move.toStr();
            printPV(depth - 1);
            unmakeMove(m);
        }
    }
}


// Prints out the board's current state
void Board::printBoard() const {
    for (int row = 7; row >= 0; row--) {
        char p;
        switch(getPiece(8 * row)) {
            case nPawn:
                p = 'p';
                break;
            case nKnight:
                p = 'n';
                break;
            case nBishop:
                p = 'b';
                break;
            case nRook:
                p = 'r';
                break;
            case nQueen:
                p = 'q';
                break;
            case nKing:
                p = 'k';
                break;
            default:
                p = '-';
                break;
        }
        if (getColor(8 * row) == nWhite) {
            p = toupper(p);
        }
        cout << p;
        for (int col = 1; col < 8; col++) {
            int sq = 8 * row + col;
            char p;
            switch(getPiece(sq)) {
                case nPawn:
                    p = 'p';
                    break;
                case nKnight:
                    p = 'n';
                    break;
                case nBishop:
                    p = 'b';
                    break;
                case nRook:
                    p = 'r';
                    break;
                case nQueen:
                    p = 'q';
                    break;
                case nKing:
                    p = 'k';
                    break;
                default:
                    p = '-';
                    break;
            }
            if (getColor(sq) == nWhite) {
                p = toupper(p);
            }
            cout << " " << p;
        }
        cout << endl;
    }
}


// Returns the evaluation of the board's score
int Board::boardScore() const {
    int endgameScore = materialCount(nWhite, true) - materialCount(nBlack,
            true);
    int openScore = materialCount(nWhite, false) - materialCount(nBlack,
            false);
    int phase = boardPhase();
    int score = ((openScore * (256 - phase)) + (endgameScore * phase)) / 256;
    score -= 12 * (getIsolatedPawns(nWhite) - getIsolatedPawns(nBlack));
    score -= 15 * (getBackwardPawns(nWhite) - getBackwardPawns(nBlack));
    score -= 18 * (getDoubledPawns(nWhite) - getDoubledPawns(nBlack));
    score += (mobilityScore(nWhite) - mobilityScore(nBlack));
    score += (passedScore(nWhite) - passedScore(nBlack));
    score += (safetyScore(nWhite) - safetyScore(nBlack));

    return (toMove == nWhite ? score : -score);
}


int Board::boardPhase() const {
    int totalPhase = 32;
    int phase = totalPhase;

    phase -= popcount(getPieces(nKnight));
    phase -= popcount(getPieces(nBishop));
    phase -= popcount(getPieces(nRook)) * 2;
    phase -= popcount(getPieces(nQueen)) * 4;

    return (phase * 256 + (totalPhase / 2)) / totalPhase;
}
// Returns the amount of material for the given color
int Board::materialCount(Color c, bool endgame) const {
    int score = 0;
    for (int p = nPawn; p <= nKing; p++) {
        Bitboard pieces = getPieces(c, (Piece)p);
        while (pieces) {
            Square sq = pop_lsb(&pieces);
            score += PieceVals[p];
            if (c == nWhite) {
                if (p == nKing && endgame) {
                    score += kingTableEndgame[sq];
                } else {
                    score += pieceTable[p][sq];
                }
            } else {
                if (p == nKing && endgame) {
                    score += kingTableEndgame[8 * (7 - sq / 8) + (sq & 7)];
                } else {
                    score += pieceTable[p][8 * (7 - sq / 8) + (sq & 7)];
                }
            }
        }
    }
    return score;
}


// Returns the number of isolated pawns of the given color
int Board::getIsolatedPawns(Color c) const {
    int count = 0;
    Bitboard pawns = getPieces(c, nPawn);
    while (pawns) {
        Square sq = pop_lsb(&pawns); 
        int file = sq & 7;
        if (file != 0) {
            if (getPieces(c, nPawn) & (0x0101010101010101 << (file - 1))) {
                continue; 
            }
        }
        if (file != 7) {
            if (getPieces(c, nPawn) & (0x0101010101010101 << (file + 1))) {
                continue; 
            }
        }
        count++;
    }
    return count;
}


// Returns the number of groups of doubled pawns for the given color
int Board::getDoubledPawns(Color c) const {
    int count = 0;
    Bitboard pawns = getPieces(c, nPawn);
    for (int i = 0; i < 8; i++) {
        Bitboard file = (0x0101010101010101 << i);
        int onFile = popcount(file & pawns);
        count += onFile - 1;
    }
    return count;
}

int Board::getBackwardPawns(Color c) const {
    Color other = (c == nWhite ? nBlack : nWhite);
    Bitboard pawns = getPieces(c, nPawn);
    Bitboard attackSpan = 0;
    Bitboard otherPawns = getPieces(other, nPawn);
    while (pawns) {
        Square square = pop_lsb(&pawns);
        attackSpan |= (pawnFrontSpan[c][square] & ~getFile(square));
    }
    Bitboard attacks = (other == nWhite ? pawnAttacksBB<nWhite>(otherPawns) :
            pawnAttacksBB<nBlack>(otherPawns));
    int up = (c == nWhite ? 8 : -8);
    return popcount((getPieces(c, nPawn) << up) & attacks & ~attackSpan);
}


// Returns whether a square has a candidate passer or not
bool Board::isPasser(Square sq) const {
    if (getPiece(sq) == nPawn) {
        Color c = getColor(sq);
        Color other = (c == nWhite ? nBlack : nWhite);

        if (!(pawnFrontSpan[c][sq] & getPieces(other, nPawn))) {
            return true;
        }

        if (getFile(sq) & pawnFrontSpan[c][sq] &
                getPieces(other, nPawn)) { // no pawn on same file
            return false; 
        }

        int up = (c == nWhite ? 8 : -8);

        sq = (Square)((int)sq + up);
        if (sq > H1 && sq < A8) {
            int support = popcount(pawnAttacks[other][sq] & getPieces(c,
                        nPawn));
            int enemies = popcount(pawnAttacks[c][sq] & getPieces(other,
                        nPawn));

            if (support >= enemies) {
                if (!(pawnFrontSpan[c][sq] & getPieces(other, nPawn))) {
                    return true;
                }
            }
            sq = (Square)((int)sq + up);
            if (sq > H1 && sq < A8) {
                return !(pawnFrontSpan[c][sq] & getPieces(other, nPawn));
            }
        }
    }
    return false;
}


// Returns the passed pawn score for the given color
int Board::passedScore(Color c) const {
    int score = 0;
    int up = (c == nWhite ? 8 : -8);
    Color other = (c == nWhite ? nBlack : nWhite);

    Bitboard pawns = getPieces(c, nPawn);
    while (pawns != 0) {
        Square square = pop_lsb(&pawns);
        if (isPasser(square)) {
            int rank = square / 8;
            if (c == nBlack) rank = 7 - rank;
            score += passedRank[rank];
            score += min(square % 8 + 1, 8 - square % 8);

            // TODO: Add static exchange evaluation for bonus
            if (getFile(square) & (getPieces(c, nRook) |
                        getPieces(c, nQueen)) & pawnFrontSpan[other][square]) {
                score += passedRank[rank] * 0.17;
            } 
            if (getFile(square) & (getPieces(other, nRook) |
                        getPieces(other, nQueen)) & pawnFrontSpan[other][square]) {
                score -= passedRank[rank] * 0.17;
            } 
            square = (Square)((int)square + up);
            while (square > H1 && square < A8) {
                if (getColor(square) == other) {
                    score -= 5;
                }
                square = (Square)((int)square + up);
            }
        }
    }
    return score;
}


// Returns the mobility score for the given color
int Board::mobilityScore(Color c) const {
    int count = 0;
    for (int sq = A1; sq <= H8; sq++) {
        if (getColor(sq) == c) {
            Piece p = getPiece(sq);
            Bitboard attacks;
            if (p == nKnight) {
                attacks = knightAttacks[sq];
            } else if (p == nBishop) {
                attacks = slidingAttacksBB<nBishop>(sq, occupiedBB);
            } else if (p == nRook) {
                attacks = slidingAttacksBB<nRook>(sq, occupiedBB);
            } else if (p == nQueen) {
                attacks = slidingAttacksBB<nQueen>(sq, occupiedBB);
            } else {
                continue;
            }
            attacks &= ~(getPieces(c, nKing) | getPieces(c, nPawn));
            if (c == nWhite) {
                attacks &= ~(shift<SOUTH_EAST>(getBlackPawns()));
                attacks &= ~(shift<SOUTH_WEST>(getBlackPawns()));
            } else if (c == nBlack) {
                attacks &= ~(shift<NORTH_EAST>(getWhitePawns()));
                attacks &= ~(shift<NORTH_WEST>(getWhitePawns()));
            }

            if (p == nKnight) {
                count += knightMob[popcount(attacks)];
            } else if (p == nBishop) {
                count += bishopMob[popcount(attacks)];
            } else if (p == nRook) {
                count += rookMob[popcount(attacks)];
            } else { // queen
                count += queenMob[popcount(attacks)];
            }
        }
    }

    return count;
}


// Returns the king safety score for the given color
int Board::safetyScore(Color c) const {
    int count = 0;
    Color opp = (c == nWhite ? nBlack : nWhite);
    int up = (c == nWhite ? 8 : -8);

    int kingSquare = lsb(getPieces(c, nKing));
    Bitboard kingZone = kingAttacks[kingSquare] | (kingAttacks[kingSquare] << up);
    kingSquare ^= sqToBB[kingSquare];

    while (kingZone) {
        Square square = pop_lsb(&kingZone);
        Bitboard attackers = getAttackers(square, opp);

        while (attackers) {
            Square attackSq = pop_lsb(&attackers);
            Piece p = getPiece(attackSq);
            if (p == nKnight || p == nBishop) {
                count += 2;
            } else if (p == nRook) {
                count += 3;
            } else if (p == nQueen) {
                count += 5;
            }
        }
    }

    return -safetyTable[count];
}