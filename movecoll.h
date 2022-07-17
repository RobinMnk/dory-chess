//
// Created by robin on 17.07.2022.
//

#include "board.h"
#include "utils.h"

#ifndef CHESSENGINE_MOVECOLL_H
#define CHESSENGINE_MOVECOLL_H

namespace MoveCollector {

    unsigned long long nodes{0};

    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    void registerMove(
            const Board &board,
            BB from, BB to
    ) {
        if constexpr (depth == 1) {
            nodes++;
        }
//        if constexpr (depth == 1) {
//            std::cout << "\t";
//        }
//        Move m {from, to, piece, flags};
//        printMove<state.whiteToMove>(m);
    }
}

#endif //CHESSENGINE_MOVECOLL_H
