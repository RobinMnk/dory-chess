#include <iostream>
#include <bitset>
#include "utils.h"
#include "movegen.h"

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    Board start = STARTBOARD;

    print_board(start);

    CheckLogicHandler clh{};
    clh.reload<STARTBOARD, STARTSTATE>();
    std::cout << std::bitset<64>(clh.getCheckMask()) << std::endl;

    MoveGenerator gen{clh};

    auto lst = gen.generate<STARTSTATE, STARTBOARD>();

    for(Move m: lst->moves) {
        if(m.from == 0 && m.to == 0) break;
        printMove<true>(m);
    }


    return 0;
}
