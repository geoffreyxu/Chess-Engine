#ifndef BOARD
#define BOARD

#include <iostream>
#include <vector>
#include <stack>
#include <sstream>
#include <cassert>
#include <algorithm>
#include "bitboard.hpp"
#include "move.hpp"

const int SEARCH_DEPTH = 6;
enum TABLE_SIZE {TABLE_SIZE = 100000};

const short pieceTable[6][64] = {
    // pawn
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    // knight
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-20,-30,-30,-20,-40,-50
    },
    // bishop
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-40,-10,-10,-40,-10,-20
    },
    // rook
    {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0,-5,
        -5,  0,  0,  0,  0,  0,  0,-5,
        -5,  0,  0,  0,  0,  0,  0,-5,
        -5,  0,  0,  0,  0,  0,  0,-5,
        -5,  0,  0,  0,  0,  0,  0,-5,
        0,  0,  0,  5,  5,  0,  0,  0
    },
    // queen
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    },
    // king
    {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    }
};

// piece square table courtesy of chess programming wikispace
const short kingTableEndgame[] =
{
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

const int knightMob[9] = {-75,-57,-9,-2,6,14,22,29,36};
const int bishopMob[14] = {-48,-20,16,26,38,51,55,63,63,68,81,81,91,98};
const int rookMob[15] = {-58,-27,-15,-10,-5,-2,9,16,30,29,32,38,46,48,58};
const int queenMob[28] = {-39,-21,3,3,14,22,28,41,43,48,56,60,60,66,67,70,71,73,79,88,88,99,102,102,106,109,113,116};
const int passedRank[7] = {0, 5, 5, 30, 70, 170, 350};

// from chess programming wikispaces
const int safetyTable[100] = {
    0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
  18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
  68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
 140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
 260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
 377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
 494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};



enum HashType {
    HASH_EXACT,
    HASH_ALPHA,
    HASH_BETA,
    HASH_NULL
};

struct HashEntry {
    unsigned long long zobrist;
    int depth;
    int score;
    bool ancient;
    HashType nodeType;
    Move move;

    HashEntry(unsigned long long zobrist, int depth, int score, bool ancient,
            HashType nodeType, Move move) {
        this->zobrist = zobrist;
        this->depth = depth;
        this->score = score;
        this->ancient = ancient;
        this->nodeType = nodeType;
        this->move = move;
    }

    HashEntry() {
        this->zobrist = 0;
        this->depth = 0;
        this->score = 0;
        this->ancient=true;
        this->nodeType=HASH_NULL;
        this->move = Move();
    }
};

class Board {
    // Holds bitboards for the different colors and types of pieces
    Bitboard pieceBB[8];    
    // Bitboard that is 1 for all the empty squares
    Bitboard emptyBB;
    // Bitboard that is 1 for all the occupied squares
    Bitboard occupiedBB;
    // En passant target square
    std::stack<Square> enPassant;
    // Holds the castling rights:
    // white CAN castle kingside
    // white CAN'T castle queenside
    // black CAN castle kingside
    // black CAN castle queenside
    std::stack<short> castling;
    // Holds the color of the side to move
    Color toMove;
    // Holds the fifty move counter
    std::stack<int> fiftyList;
    // holds the list of captured pieces
    std::stack<Piece> capturedList;
    // holds the full move counter
    int fullMove;
    // holds the zobrist keys
    std::vector<unsigned long long> zobrist;
    // holds the transposition table
    HashEntry transTable[TABLE_SIZE];
public:
    // Holds the killer moves list
    Move killerMoves[SEARCH_DEPTH + 1][2];
    // holds the list of moves
    std::vector<Move> moveList;
    /**
     * Constructs a new Board object to the starting chess position.
     */
    Board();

    /**
     * Constructs a new Board object to the given position
     *
     * @param FEN the desired starting position in FEN form
     */
    Board(std::string FEN);

    // Sets the board's Zobrist key to the one for the given position
    void setZobrist();

    // Sets the board's to the state desribed by the FEN
    void setPosition(std::string FEN);

    // Returns the board's FEN string
    std::string getFEN() const;

    // Returns pieces of the given piece type
    Bitboard getPieces(Piece pt) const;

    // Returns pieces of the given color
    Bitboard getPieces(Color ct) const;

    // Returns pieces of the given piece and color
    Bitboard getPieces(Color ct, Piece pt) const;

    // Returns the piece on a given square
    Piece getPiece(int sq) const;

    // Returns the color of the piece on the given square
    Color getColor(int sq) const;

    // Returns a bitboard representing the file the current square is on.
    Bitboard getFile(Square sq) const;

    // Returns a bitboard representing the rank the current square is on.
    Bitboard getRank(Square sq) const;

    // Returns a bitboard holding the locations of the white pawns
    Bitboard getWhitePawns() const;

    // Returns a bitboard holding the locations of the black pawns
    Bitboard getBlackPawns() const;

    // Returns occupied bitboard
    Bitboard getOccupied() const;

    // Returns bitboard of empty pieces
    Bitboard getEmpty() const;

    // Returns target of en passant
    Square enPassantTarget() const;
    
    // Returns the castling rights of the board
    short getCastlingRights() const;

    // Returns the color of the side to move
    Color getToMove() const;

    // Returns the zobrist hash key
    unsigned long long getZobrist() const;

    // Returns the fifty move counter
    int getFiftyCount() const;

    // Returns whether a square is attacked by a given side
    bool attacked(int square, Color side) const;

    // Returns a bitboard holding the pieces attacking the square of the given
    // color
    Bitboard getAttackers(Square sq, Color c) const;

    // Returns a color's least valuable attacker of a square
    Square lva(Square sq, Color side) const;

    // Returns whether the player to move is in check or not
    bool inCheck() const;

    // Returns whether the player is in double check our not
    bool doubleCheck() const;

    // Returns a bitboard holding the pieces checking the king
    Bitboard getCheckers() const;

    // returns all pieces that are pinned to a square by a piece of either color
    Bitboard pinnedPieces(Bitboard pinners, Square sq) const;

    // Makes a legal move on the chessboard
    void makeMove(Move m);

    // Undoes the last move
    void unmakeMove(Move m);

    // Makes a null move (switches color) for the current position
    void makeNullMove();

    // Unmakes a null move for the current position
    void unmakeNullMove();

    // Checks if a pseudo-legal move is legal
    bool isLegal(Move m) const;

    // Gets an entry from the transposition table
    HashEntry getTransTable(int key) const;

    // Updates an entry in the transposition table
    void setTransTable(int key, HashEntry entry);

    // Returns whether this position has been repeated at some point
    bool isRep();

    // Prints out the principal variation up to a given depth
    void printPV(int depth);

    // Prints out the board's current state
    void printBoard() const;
    // Returns the evaluation of the board's score
    int boardScore() const;
    // Returns an integer representing the game phase
    int boardPhase() const;
    // Returns the amount of material for the given color
    int materialCount(Color c, bool endgame) const;
    // Returns the number of isp
    int getIsolatedPawns(Color c) const;
    // Returns the number of isolated pawns of the given color
    int getDoubledPawns(Color c) const;
    // Returns the number of backward pawns of the given color
    int getBackwardPawns(Color c) const;
    // Returns whether a square has a candidate passer or not
    bool isPasser(Square sq) const;
    // Returns the passed pawn score for the given color
    int passedScore(Color c) const;
    // Returns the mobility score for the given color
    int mobilityScore(Color c) const;
    // Returns the king safety score for the given color
    int safetyScore(Color c) const;
};

#endif // #ifndef BOARD
