#include <iostream>
#include <string>
#include "board.hpp"
#include "search.hpp"
#include <thread>
#include <sstream>

using namespace std;

class UCI {
    unsigned int wtime;
	unsigned int btime;
	unsigned int winc;
	unsigned int binc;
	unsigned int movestogo;
    Board b;
    SearchInfo info;
    thread thr;
public:
    UCI();
    void loop();
    Move stringToMove(string s);
    void findMove(int max);
};