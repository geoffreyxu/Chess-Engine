#ifndef MOVEGEN_HPP
#define MOVEGEN_HPP

#include "board.hpp"
#include "move.hpp"
#include "bitboard.hpp"
#include <vector>
#include <algorithm>

using namespace std;


enum MoveType {
    ALL,
    EVASIONS, 
    CAPTURES,
    QUIET
};

template<Color c, MoveType mv>
void getPawnMoves(vector<Move> &moveList, Board &b, Bitboard targets) {
    Bitboard pawns = b.getPieces(c, nPawn);
    Bitboard empty = b.getEmpty();
    Bitboard other = (c == nWhite ? b.getPieces(nBlack) : b.getPieces(nWhite));
    Bitboard fourthRank = (c == nWhite ? Rank4 : Rank5);
    Square enPassant = b.enPassantTarget();

    constexpr Direction up = (c == nWhite ? NORTH : SOUTH);
    constexpr Direction upRight = (c == nWhite ? NORTH_EAST : SOUTH_WEST);
    constexpr Direction upLeft = (c == nWhite ? NORTH_WEST : SOUTH_EAST);

    Bitboard singleMoves = shift<up>(pawns) & empty;
    Bitboard doubleMoves = (shift<up>(singleMoves) & fourthRank) & empty;
    Bitboard attacksLeft = shift<upLeft>(pawns);
    Bitboard attacksRight = shift<upRight>(pawns);


    singleMoves = (mv == EVASIONS ? singleMoves & targets : singleMoves);
    doubleMoves = (mv == EVASIONS ? doubleMoves & targets : doubleMoves);
    attacksLeft = (mv == EVASIONS ? attacksLeft & targets : attacksLeft);
    attacksRight = (mv == EVASIONS ? attacksRight & targets : attacksRight);

    // en passant
    if (mv != QUIET) {
        if (enPassant != SQ_NONE) {
            if (sqToBB[enPassant] & attacksLeft) {
                moveList.push_back(Move(enPassant - upLeft, enPassant, 5)); 
            }
            if (sqToBB[enPassant] & attacksRight) {
                moveList.push_back(Move(enPassant - upRight, enPassant, 5)); 
            }
        }
    }

    attacksLeft &= other;
    attacksRight &= other;

    // single pawn moves
    if (mv != CAPTURES) {
        while (singleMoves) {
            int index = pop_lsb(&singleMoves);
            if (index >= A8 || index <= H1) { // promotion
                if (mv != QUIET) {
                    for (int flag = 8; flag <= 11; flag++) {
                        moveList.push_back(Move(index - up, index, flag));
                    }
                }
            } else {
                moveList.push_back(Move(index - up, index, 0));
            }
        }

        while (doubleMoves) {
            int index = pop_lsb(&doubleMoves);
            moveList.push_back(Move(index - 2 * up, index, 1));
        }
    }

    if (mv != QUIET) {
        while (attacksLeft) {
            int index = pop_lsb(&attacksLeft);
            if (index >= A8 || index <= H1) {
                for (int flag = 12; flag <= 15; flag++) {
                    moveList.push_back(Move(index - upLeft, index, flag));
                }
            } else {
                moveList.push_back(Move(index - upLeft, index, 4));
            }
        }

        while (attacksRight) {
            int index = pop_lsb(&attacksRight);
            if (index >= A8 || index <= H1) {
                for (int flag = 12; flag <= 15; flag++) {
                    moveList.push_back(Move(index - upRight, index, flag));
                }
            } else {
                moveList.push_back(Move(index - upRight, index, 4));
            }
        }
    }
}

template<Color c, Piece p, MoveType mv>
void getSlidingMoves(vector<Move> &moveList, Board &b, Bitboard targets) {
    Bitboard pieces = b.getPieces(c, p);
    Bitboard occupied = b.getOccupied();
    Bitboard other = (c == nWhite ? b.getPieces(nBlack) : b.getPieces(nWhite));

    while (pieces) {
        int square = pop_lsb(&pieces);
        // automatically finds proper valid moves
        Bitboard attacks = slidingAttacksBB<p>(square, occupied);
        attacks = (mv == EVASIONS ? attacks & targets : attacks);
        while (attacks) {
            int attackSquare = pop_lsb(&attacks);
            if ((sqToBB[attackSquare] & other) && mv != QUIET) { // other piece there
                moveList.push_back(Move(square, attackSquare, 4)); 
            } else if ((sqToBB[attackSquare] & ~occupied) && mv != CAPTURES) { // not a capture
                moveList.push_back(Move(square, attackSquare, 0)); 
            }
        }
    }
}

template<Color c, Piece p, MoveType mv>
void getMoves(vector<Move> &moveList, Board &b, Bitboard targets) {
    Bitboard pieces = b.getPieces(c, p); 
    Bitboard occupied = b.getOccupied();
    Bitboard other = (c == nWhite ? b.getPieces(nBlack) : b.getPieces(nWhite));
    
    while (pieces) {
        int square = pop_lsb(&pieces);
        Bitboard attacks;
        if (p == nKnight) {
            attacks = knightAttacks[square];
        } else { // king
            attacks = kingAttacks[square];
        }

        attacks = (mv == EVASIONS ? attacks & targets : attacks);
        while (attacks) {
            int attackSquare = pop_lsb(&attacks);
            if ((sqToBB[attackSquare] & other) && mv != QUIET) { // other piece there
                moveList.push_back(Move(square, attackSquare, 4)); 
            } else if ((sqToBB[attackSquare] & ~occupied) && mv != CAPTURES) { // not a capture
                moveList.push_back(Move(square, attackSquare, 0)); 
            }
        }
    }
}

template<Color c>
void getCastleMoves(vector<Move> &moveList, Board &b, bool
        kingSide) {
    short castling = b.getCastlingRights();      
    Square start = (c == nWhite) ? E1 : E8;
    Square end = kingSide ? ((c == nWhite) ? G1 : G8) : ((c == nWhite) ? C1
            : C8); 
    Square rookSq = kingSide ? ((c == nWhite) ? H1 : H8) : ((c == nWhite) ? A1
            : A8); 
    int step = (kingSide ? WEST : EAST);

    Color other = (c == nWhite) ? nBlack : nWhite;
    if ((castling >> (2 * other + kingSide)) & 1) {
        for (int i = end; i != start; i+=step) {
            if (b.attacked((Square)i, other)) {
                return;
            }
        }
        if (!(betweenBB[start][rookSq] & b.getOccupied())) {
            moveList.push_back(Move(start, end, 2 + !kingSide));
        }
    }
}

template<Color c>
inline void getAllEvasions(vector<Move> &moveList, Board& b, Bitboard targets) {
    getPawnMoves<c, EVASIONS>(moveList, b, targets);
    getMoves<c, nKnight, EVASIONS>(moveList, b, targets);
    getSlidingMoves<c, nBishop, EVASIONS>(moveList, b, targets);
    getSlidingMoves<c, nRook, EVASIONS>(moveList, b, targets);
    getSlidingMoves<c, nQueen, EVASIONS>(moveList, b, targets);
}

template<Color c>
inline void getCaptures(vector<Move> &moveList, Board& b) {
    getPawnMoves<c, CAPTURES>(moveList, b, 0);
    getMoves<c, nKnight, CAPTURES>(moveList, b, 0);
    getMoves<c, nKing, CAPTURES>(moveList, b, 0);
    getSlidingMoves<c, nBishop, CAPTURES>(moveList, b, 0);
    getSlidingMoves<c, nRook, CAPTURES>(moveList, b, 0);
    getSlidingMoves<c, nQueen, CAPTURES>(moveList, b, 0);
    getSlidingMoves<c, nKing, CAPTURES>(moveList, b, 0);
}

// get moves to get out of check
template<Color c>
void getEvasions(vector<Move> &moveList, Board& b) {
    Color other = (c == nWhite) ? nBlack : nWhite;
    Bitboard otherBB = b.getPieces(other);

    // find list of sliding checkers
    Bitboard checkers = b.getCheckers();
    Bitboard sliders = ~b.getPieces(other, nPawn) & ~b.getPieces(other, nKnight);
    sliders &= checkers;
    Bitboard sliderAttacks = 0;

    int kingSquare = lsb(b.getPieces(c, nKing));

    while (sliders) {
        int attackSq = pop_lsb(&sliders);
        sliderAttacks |= lineBB[attackSq][kingSquare] ^ sqToBB[attackSq];
    }

    // generate list of king evading moves
    Bitboard kingMoves = kingAttacks[kingSquare] & ~sliderAttacks;
    while (kingMoves) {
        int square = pop_lsb(&kingMoves);
        if (sqToBB[square] & otherBB) {
            moveList.push_back(Move(kingSquare, square, 4)); 
        } else {
            moveList.push_back(Move(kingSquare, square, 0)); 
        }
    }

    if (b.doubleCheck()) {
        return;
    }

    // We know there is one checker now.
    int checkSq = lsb(checkers);
    Bitboard targets = betweenBB[checkSq][kingSquare] | sqToBB[checkSq];
    getAllEvasions<c>(moveList, b, targets);
}

template<Color c>
inline void getPseudoLegalMoves(vector<Move> &moveList, Board& b) {
    getPawnMoves<c, ALL>(moveList, b, 0);
    getSlidingMoves<c, nBishop, ALL>(moveList, b, 0);
    getSlidingMoves<c, nRook, ALL>(moveList, b, 0);
    getSlidingMoves<c, nQueen, ALL>(moveList, b, 0);
    getMoves<c, nKnight, ALL>(moveList, b, 0);
    getMoves<c, nKing, ALL>(moveList, b, 0);
    getCastleMoves<c>(moveList, b, true);
    getCastleMoves<c>(moveList, b, false);
}

template<Color c>
inline void getAllMoves(vector<Move> &moveList, Board& b) {
    if (b.inCheck()) {
        getEvasions<c>(moveList, b);
    } else {
        getPseudoLegalMoves<c>(moveList, b);
    }
}

template<Color c>
inline void getLegalMoves(vector<Move> &moveList, Board& b) {
    c == nWhite ? getAllMoves<nWhite>(moveList, b) :
        getAllMoves<nBlack>(moveList, b);
    vector<Move>::iterator it = moveList.begin();
    while (it != moveList.end()) {
        if (!b.isLegal(*it)) {
            it = moveList.erase(it);
        } else {
            it++;
        }
    }
}


#endif