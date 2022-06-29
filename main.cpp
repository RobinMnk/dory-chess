#include <iostream>
#include "board.h"
#include "utils.h"
#include "piecesteps.h"

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    Board start = STARTBOARD;

    print_board(start);

    return 0;
}
