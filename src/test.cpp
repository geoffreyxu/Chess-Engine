#include "board.hpp"
#include "search.hpp"
#include "uci.hpp"

int main() {
    UCI uci = UCI();
	uci.loop();

    return 0;
}