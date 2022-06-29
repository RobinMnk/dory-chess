//
// Created by Robin on 29.06.2022.
//

#include <array>
#include "chess.h"

#ifndef CHESSENGINE_PIECESTEPS_H
#define CHESSENGINE_PIECESTEPS_H

namespace PieceSteps {

    static const std::array<int, 8> directions{8, 9, 1, -7, -8, -9, -1, 7};
    static const std::array<int, 4> diagonal{1, 3, 5, 7}, straight{0, 2, 4, 6};

    static std::array<std::array<BB, 64>, 8> LINES{};
    static std::array<std::array<BB, 64>, 64> FROM_TO{};

    template<bool diag>
    static std::array<BB, 64> STEPS{};


    static std::array<BB, 64> KNIGHT_MOVES{}, KING_MOVES{};


    static constexpr int manhattan(int x1, int y1, int x2, int y2) {
        return abs(x2 - x1) + abs(y2 - y1);
    }
    static constexpr int manhattan(int index1, int index2) {
        return manhattan(
            file(index1),
            rank(index1),
            file(index2),
            rank(index2)
        );
    }

    template<bool diag>
    static constexpr void calculate_lines(int i) {
        BB board = 0;
        int j;
        int manhattan_dist = diag ? 2 : 1;
        for(int id: diag ? diagonal : straight) {
            int off = directions[id];
            board = 0;
            j = i + off;
            while(0 <= j && j < 64 && manhattan(j-off, j) == manhattan_dist){
                board |= 1 << j;
                FROM_TO[i][j] = board;
                j += off;
            }
            if(board) {
                STEPS<diag>[i] = board;
                LINES[i][id] = board;
            }
        }
    }

    static constexpr void addKnightMoves(int index) {
        BB board = 0;
        for(int off: std::array<int, 8>{-17, -15, -6, 10, 17, 15, 6, -10}) {
            int to = index + off;
            if(0 <= to && to < 64 && manhattan(index, to) == 3) {
                board |= 1 << to;
            }
        }
        KNIGHT_MOVES[index] = board;
    }

    static constexpr void addKingMoves(int index) {
        BB board = 0;
        for(int off: std::array<int, 8>{-9, -8, -7, -1, 1, 7, 8, 9}) {
            int to = index + off;
            if(0 <= to && to < 64 && manhattan(index, to) <= 2) {
                board |= 1 << to;
            }
        }
        KING_MOVES[index] = board;
    }


    static constexpr void load() {
        for(int i = 0; i < 64; i++) {
            calculate_lines<true>(i);
            calculate_lines<false>(i);
            addKnightMoves(i);
            addKingMoves(i);
        }
    }



}

#endif //CHESSENGINE_PIECESTEPS_H
