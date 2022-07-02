//
// Created by Robin on 29.06.2022.
//

#include <array>
#include "chess.h"

#ifndef CHESSENGINE_PIECESTEPS_H
#define CHESSENGINE_PIECESTEPS_H

// used to terminate arrays that represent list of squares
static const uint8_t END_OF_ARRAY = 0x7f;

namespace PieceSteps {

    static const std::array<int, 8> directions{8, 9, 1, -7, -8, -9, -1, 7};
    static const std::array<int, 4> diagonal{1, 3, 5, 7}, straight{0, 2, 4, 6};
    static const int DIR_LEFT = 6, DIR_RIGHT = 2;

    static std::array<std::array<BB, 8>, 64> LINES{};

    static std::array<std::array<BB, 64>, 64> FROM_TO{};

    template<bool>
    static std::array<std::array<std::array<uint8_t, 8>, 4>, 64> STEPS{};

    template<bool>
    static std::array<BB, 64> PAWN_CAPTURES{};

    static std::array<BB, 64> KNIGHT_MOVES{}, KING_MOVES{};


    static constexpr int manhattan(int x1, int y1, int x2, int y2) {
        return abs(x2 - x1) + abs(y2 - y1);
    }
    static constexpr int manhattan(int index1, int index2) {
        return manhattan(
            fileOf(index1),
            rankOf(index1),
            fileOf(index2),
            rankOf(index2)
        );
    }

    template<bool diag>
    static constexpr void calculate_lines(int i) {
        int j;
        int d{0}, x{0};
        int manhattan_dist = diag ? 2 : 1;
        for(int id: diag ? diagonal : straight) {
            int off = directions[id];
            BB board = 0;
            j = i + off;
            while(0 <= j && j < 64 && manhattan(j-off, j) == manhattan_dist){
                board = withBit(board, j);
                FROM_TO[i][j] = board;
                STEPS<diag>[i][d][x++] = j;
                j += off;
            }
            LINES[i][id] = board;
            STEPS<diag>[i][d][x] = END_OF_ARRAY;
            d++;
            x = 0;
        }
    }

    static constexpr void addKnightMoves(int index) {
        BB board{0};
        for(int off: std::array<int, 8>{-17, -15, -6, 10, 17, 15, 6, -10}) {
            int to = index + off;
            if(0 <= to && to < 64 && manhattan(index, to) == 3) {
                setBit(board, to);
            }
        }
        KNIGHT_MOVES[index] = board;
    }

    static constexpr void addKingMoves(int index) {
        BB board{0};
        for(int off: std::array<int, 8>{-9, -8, -7, -1, 1, 7, 8, 9}) {
            int to = index + off;
            if(0 <= to && to < 64 && manhattan(index, to) <= 2) {
                setBit(board, to);
            }
        }
        KING_MOVES[index] = board;
    }


    static constexpr void addPawnCaptures(int index) {
        std::array<int, 2> attack{7, 9};
        int file = fileOf(index);

        // white attacks
        BB board{0};
        for(int off: attack) {
            int newFile = fileOf(index + off);
            if(index+off >= 0 && index+off < 64 && 0 <= newFile && newFile < 8 && abs(newFile - file) == 1) {
                setBit(board, index + off);
            }
        }
        PAWN_CAPTURES<true>[index] = board;

        // black attacks
        board = 0;
        for(int off: attack) {
            off *= -1;
            int newFile = fileOf(index + off);
            if(index+off >= 0 && index+off < 64 && 0 <= newFile && newFile < 8 && abs(newFile - file) == 1) {
                setBit(board, index + off);
            }
        }
        PAWN_CAPTURES<false>[index] = board;
    }

    static constexpr void load() {
        for(int i = 0; i < 64; i++) {
            calculate_lines<true>(i);
            calculate_lines<false>(i);
            addKnightMoves(i);
            addKingMoves(i);
            addPawnCaptures(i);
        }
    }

    template<Board board, bool whiteToMove, bool diag, bool xrayking>
    static constexpr BB slideMask(int index) {
        BB mask = 0;
        for(auto line: STEPS<diag>[index]) {
            bool breakNext = false;
            for(uint8_t sq: line) {
                if(sq == END_OF_ARRAY) break;
                BB mk = newMask(sq);
                mask |= mk;
                if(breakNext) break;
                if(board.occ() & mk) {
                    if(xrayking && mk == board.king<whiteToMove>()) breakNext = true;
                    else break;
                }
            }
        }
        return mask;
    }
}

#endif //CHESSENGINE_PIECESTEPS_H
