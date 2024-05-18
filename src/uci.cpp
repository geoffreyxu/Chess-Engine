
#include "uci.hpp"

using namespace std;

UCI::UCI() {
    wtime = 0;
}
void UCI::loop() {
    string start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    initBitboards();

    //b.printBoard();
    std::string line;
    std::string token;
    cout.setf (ios::unitbuf);
    while (getline(cin, line)) {
        istringstream is(line);
        token.clear();
        is >> skipws >> token;

        if (token == "uci") {
            cout << "id name Engine" << endl;
            cout << "id author Brock Grassy" << endl;
            cout << "uciok" << endl;
        } else if (token == "isready") {
            cout << "readyok" << endl;
        } else if (token == "ucinewgame") {
            b.setPosition(start);
        } else if (token == "position") {
            is >> token;
            if (token == "startpos") {
                b.setPosition(start);
            } else if (token == "fen") {
                string fen;
                while (is >> token && token != "moves") {
                    fen += token + " ";
                }
                fen.pop_back();
                b.setPosition(fen);
                b.printBoard();
            }

            while (is >> token) {
                if (token != "moves") { // make move
                    Move m = stringToMove(token);
                    b.makeMove(m);
                }
            }
        } else if (token == "go") {
            if (info.stopped) {
                int max = 9;
                info.duration = 0;
                info.stopped = false;
                while (is >> token) {
                    if (token == "depth") {
                        is >> max;
                    } if (token == "movetime") {
                        is >> info.duration;
                    } if (token == "infinite") {
                        max = 1000;
                    }
                }
                thread th1(&UCI::findMove, this, max);
                th1.detach();
            }

        } else if (token == "stop") {
            info.stopped = true;
        } else if (token == "print") {
            b.printBoard();
            cout << endl;
        } else if (token == "quit") {
            break;
        } 
    }
}

void UCI::findMove(int max) {
    vector<Move> moveList;
    Move bestMove;
    Search search(&info);

    for (int depth = 1; depth <= max; depth++) {
        info.startTime = chrono::high_resolution_clock::now();
        info.nodes = 0;

        vector<Move> moves;
        vector<MoveData> moveList;

        b.getToMove() == nWhite ? getAllMoves<nWhite>(moves, b) : getAllMoves<nBlack>(moves, b);
        search.orderMoves(b, moves, moveList, -1);

        info.depth = depth;
        info.nodes = 0;
        int score = search.negamaxRoot(b, depth, -MAX_VALUE, MAX_VALUE);
        bestMove = search.bestMove;

        if (info.stopped) {
            break;
        }

        auto time = chrono::high_resolution_clock::now();
        auto dur = time - info.startTime;
        cout << "info depth " << depth << " nodes " << info.nodes << " score cp ";
        cout << score << " pv";
        b.printPV(depth);
        if (chrono::duration_cast<std::chrono::milliseconds>(dur).count() != 0) {
            cout << " nps " << (int)(0.5 + info.nodes * 1000.0 /
                    chrono::duration_cast<std::chrono::milliseconds>(dur).count());
        }
        cout << endl; 
    }
    b.makeMove(bestMove);
    cout << endl;
    b.printBoard();
    cout << "bestmove " << bestMove.toStr() << endl;
    info.stopped = true;
}

Move UCI::stringToMove(string s) {
    vector<Move> moveList;     
    b.getToMove() == nWhite ? getAllMoves<nWhite>(moveList, b) :
        getAllMoves<nBlack>(moveList, b);

    for (Move m : moveList) {
        if (s == m.toStr()) {
            return m;
        }
    }
    cout << "INVALID" << endl;
    return Move();

}
