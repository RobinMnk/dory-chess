#include <iostream>
#include <bitset>
#include "utils.h"
#include "movegen.h"

int main() {
    std::cout << "Chess Engine" << std::endl;

    PieceSteps::load();

    constexpr Board board = STARTBOARD;
    constexpr State state = STARTSTATE;

    MoveGenerator gen{};
    print_board(board);

    auto lst = gen.generate<board, state>();

    std::cout << lst->size() << " legal moves:" << std::endl;
    for(Move m: lst->moves) {
        if(m.from == 0 && m.to == 0) break;
        printMove<true>(m);
    }


    std::cout << "Applying the 10th move" << std::endl;
    Move move = lst->moves[9];

    return 0;
}
