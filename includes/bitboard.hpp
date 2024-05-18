#ifndef BITBOARD_H
#define BITBOARD_H

#include "MagicMoves.hpp"
#include <string>
#include <bitset>

typedef unsigned long long Bitboard;

extern const Bitboard AFile;
extern const Bitboard BFile;
extern const Bitboard CFile;
extern const Bitboard DFile;
extern const Bitboard EFile;
extern const Bitboard FFile;
extern const Bitboard GFile;
extern const Bitboard HFile;

extern const Bitboard Rank1;
extern const Bitboard Rank2;
extern const Bitboard Rank3;
extern const Bitboard Rank4;
extern const Bitboard Rank5;
extern const Bitboard Rank6;
extern const Bitboard Rank7;
extern const Bitboard Rank8;

extern Bitboard pawnAttacks[2][64];
extern Bitboard pawnFrontSpan[2][64];
extern Bitboard knightAttacks[64];
extern Bitboard kingAttacks[64];
extern Bitboard betweenBB[64][64];
extern Bitboard lineBB[64][64];

// Holds mapping from square number to corresponding bitboard.
extern const Bitboard sqToBB[64];

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6, 
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQ_NONE
};

enum Piece {
    nPawn,
    nKnight,
    nBishop,
    nRook,
    nQueen,
    nKing, 
    PIECE_NONE
};

extern const int PieceVals[6];


enum Color {
    nWhite,
    nBlack,
    COLOR_NONE
};

enum Direction {
    NORTH = 8, 
    SOUTH = -8, 
    EAST = 1, 
    WEST = -1,
    NORTH_EAST = 9, 
    NORTH_WEST = 7, 
    SOUTH_EAST = -7, 
    SOUTH_WEST = -9
};


// Initialize key bitboard constants
void initBitboards();

// Returns the least significant bit of the bitboard
Square lsb(Bitboard b); 

// Returns the most significant bit of the bitboard
Square msb(Bitboard b);

// Pops and returns the least significant bit of the bitboard
Square pop_lsb(Bitboard* b);

// returns the number of set bits in the bitboard
int popcount(Bitboard b);


// Shifts the bitboard in a given direction
template<Direction D>
constexpr Bitboard shift(Bitboard b) {
    return (D == NORTH ? b << 8 : 
           (D == SOUTH ? b >> 8 :
           (D == EAST ? (b & ~HFile) << 1 : 
           (D == WEST ? (b & ~AFile) >> 1 :
           (D == NORTH_WEST ? (b & ~AFile) << 7 :
           (D == NORTH_EAST ? (b & ~HFile) << 9 :
           (D == SOUTH_EAST ? (b & ~HFile) >> 7 :
           (D == SOUTH_WEST ? (b & ~AFile) >> 9 :
           b))))))));
}


// returns the pawn attacks for a bitboard of pawns and color
template<Color C>
constexpr Bitboard pawnAttacksBB(Bitboard pawns) {
    return (C == nWhite ? shift<NORTH_WEST>(pawns) | shift<NORTH_EAST>(pawns) :
            shift<SOUTH_EAST>(pawns) | shift<SOUTH_WEST>(pawns));
}


// Returns the sliding piece attacks for given piece from a square, given
// occupied squares
template<Piece P>
constexpr Bitboard slidingAttacksBB(int sq, Bitboard occupied) {
    return (P == nBishop ? Bmagic(sq, occupied) : (P == nRook ? Rmagic(sq, occupied) :
           (P == nQueen ? Bmagic(sq, occupied) | Rmagic(sq, occupied) : 0)));
}


// Returns all squares attacked by all pieces of a certain type
template<Piece P>
inline Bitboard allSlidingAttacks(Bitboard pieces, Bitboard occupied) {
    Bitboard attacks = 0; 
    while (pieces != 0) {
        int square = pop_lsb(&pieces);
        attacks |= slidingAttacksBB<P>(square, occupied);
    }
    return attacks;
}


// Returns a bitboard of squares attacked by knights placed on given squares
inline Bitboard knightAttacksBB(Bitboard knights) {
    return ((knights << 17) & ~AFile) | ((knights << 10) & ~AFile &~BFile) | 
           ((knights >>  6) & ~AFile & ~BFile) | ((knights >> 15) & ~AFile)
           | ((knights << 15) & ~HFile) | ((knights <<  6) & ~GFile & ~HFile)
           | ((knights >> 10) & ~GFile & ~HFile)
           | ((knights >> 17) & ~HFile);

}


// Returns a bitboard of squares attacked by kings placed on given squares
inline Bitboard kingAttacksBB(Bitboard king) {
    return ((king << 1) & ~HFile) | ((king >> 1) & ~AFile)
    | ((king << 8) | (king >> 8)) | ((king << 8) | (king >> 8));
}

#endif /* ifndef BITBOARD_H */