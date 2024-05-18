#ifndef SEARCH_HPP
#define SEARCH_HPP
#include <algorithm>
#include <utility>
#include "board.hpp"
#include "movegen.hpp"
#include <chrono>

extern const int MAX_VALUE;
extern const int MATE_VALUE;

struct MoveData {
    Move move;
    int score;

    MoveData(int score, Move m) {
        this->move = m;
        this->score = score;
    }
    
    MoveData() {
        this->move = Move();
        this->score = 0;
    }
};

struct SearchInfo {
	chrono::high_resolution_clock::time_point startTime;
	chrono::high_resolution_clock::time_point time;
    int depth;
    long duration; // in ms
    int nodes;
    bool infinite;
    bool stopped;

    SearchInfo() {
        depth = 0;
        duration = 0;
        nodes = 0;
        infinite = false;
        stopped = true;
    }
};

struct sortMoves {
    bool operator()(MoveData const &a, MoveData const &b) { 
            return a.score > b.score;
    }
};

class Search {
    SearchInfo* info;
public:
    // Constructs a new search object
    Search(SearchInfo* info);

    // Holds the best move for the search
    Move bestMove; 

    int negamax(Board &b, int depth, int alpha, int beta, bool pv, bool
            nullOkay);

    int negamaxRoot(Board &b, int depth, int alpha, int beta);

    // Performs quiescence search on the given board
    int quiesce(Board &b, int alpha, int beta);

    // Orders the moves in the given move list
    void orderMoves(Board& b, std::vector<Move>& moveList, std::vector<MoveData>& moveScores, int ply);
};
#endif /*SEARCH_HPP*/