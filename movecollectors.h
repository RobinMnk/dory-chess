//
// Created by robin on 17.07.2022.
//

#ifndef CHESSENGINE_MOVECOLLECTORS_H
#define CHESSENGINE_MOVECOLLECTORS_H

#include "movegen.h"

template<bool print>
class MoveCollectorStandard {
public:
    static unsigned long long nodes;

    template<State state, int depth>
    static void generateGameTree(Board& board) {
        nodes = 0;
        build<state, depth>(board);
    }

    template<State state, int depth>
    static void build(Board& board) {
        if constexpr (depth > 0) {
            MoveGenerator<MoveCollectorStandard<print>>::template generate<state, depth>(board);
        }
    }

    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {
        if constexpr (depth == 1) {
            nodes++;
        }

        if constexpr (print) {
            for(int i = 0; i < 3 - depth; i++){
                std::cout << "\t";
            }
            Move m {from, to, piece, flags};
            printMove<state.whiteToMove>(m);
        }
    }

    template<State nextState, int depth>
    static void next(Board& nextBoard) {
        build<nextState, depth-1>(nextBoard);
    }
};

template<bool print>
unsigned long long MoveCollectorStandard<print>::nodes{0};

#endif //CHESSENGINE_MOVECOLLECTORS_H
