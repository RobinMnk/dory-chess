#include <iostream>
#include "board.h"
#include "utils.h"
#include "piecesteps.h"
#include "checklogichandler.h"

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    Board start = STARTBOARD;

    print_board(start);

    CheckLogicHandler clh{75};
    clh.reload<STARTBOARD, STARTSTATE>();
    std::cout << clh.checkMask << std::endl;

    std::cout << (start.bQueens & clh.checkMask) << std::endl;

    return 0;
}
