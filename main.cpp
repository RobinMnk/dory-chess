#include <iostream>
#include "utils.h"
#include "movegen.h"

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    Board start = STARTBOARD;

    print_board(start);

    CheckLogicHandler clh{};
    clh.reload<STARTBOARD, STARTSTATE>();
    std::cout << clh.checkMask << std::endl;

    std::cout << (start.bQueens & clh.checkMask) << std::endl;

    MoveGenerator gen{clh};

    auto lst = gen.generate<STARTSTATE, STARTBOARD>();

    for(Move m: lst->moves) {
        if(m.from == 0 && m.to == 0) break;
        printMove<true>(m);
//        std::cout << ('0' + m.piece) << ": " << (m.from) << " -> " << singleBitOf(m.to) << std::endl;
    }


    return 0;
}
