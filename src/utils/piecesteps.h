//
// Created by Robin on 29.06.2022.
//

#include <array>
#include "../core/chess.h"

#ifndef DORY_PIECESTEPS_H
#define DORY_PIECESTEPS_H

// used to terminate arrays that represent list of squares
const uint8_t END_OF_ARRAY = 0x7f;

namespace PieceSteps {

    constexpr std::array<int, 8> directions{8, 9, 1, -7, -8, -9, -1, 7};
    constexpr std::array<int, 4> diagonal{1, 3, 5, 7}, straight{0, 2, 4, 6};
    constexpr int DIR_LEFT = 6, DIR_RIGHT = 2;

    std::array<std::array<BB, 8>, 64> LINES{};

    std::array<std::array<BB, 64>, 64> FROM_TO{};

    template<bool>
    std::array<std::array<std::array<uint8_t, 8>, 4>, 64> STEPS{};

    std::array<BB, 64> KNIGHT_MOVES{}, KING_MOVES{};

    bool loaded{false};

    int manhattan(int x1, int y1, int x2, int y2) {
        return abs(x2 - x1) + abs(y2 - y1);
    }
    int manhattan(int index1, int index2) {
        return manhattan(
            fileOf(index1),
            rankOf(index1),
            fileOf(index2),
            rankOf(index2)
        );
    }

    template<bool diag>
    void calculate_lines(int i) {
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

    void addKnightMoves(int index) {
        BB board{0};
        for(int off: std::array<int, 8>{-17, -15, -6, 10, 17, 15, 6, -10}) {
            int to = index + off;
            if(0 <= to && to < 64 && manhattan(index, to) == 3) {
                setBit(board, to);
            }
        }
        KNIGHT_MOVES[index] = board;
    }

    void addKingMoves(int index) {
        BB board{0};
        for(int off: std::array<int, 8>{-9, -8, -7, -1, 1, 7, 8, 9}) {
            int to = index + off;
            if(0 <= to && to < 64 && manhattan(index, to) <= 2) {
                setBit(board, to);
            }
        }
        KING_MOVES[index] = board;
    }

    template<bool diag>
    BB slideMaskSlow(BB occ, int index) {
        BB mask = 0ull;
        for(auto line: STEPS<diag>.at(index)) {
            for(uint8_t sq: line) {
                if(sq == END_OF_ARRAY) break;
                setBit(mask, sq);
                if(hasBitAt(occ, sq)) break;
            }
        }
        return mask;
    }

    std::array<BB, 140000> sliderAttackBB;

    template<bool>
    std::array<BB, 64> arrMask;

    std::array<int, 64> arrRookBase, arrBishopBase;
    BB inside = ~(fileA | fileH | rank1 | rank8);

    template<bool diag>
    BB slideMask(BB occ, int sq) {
        if constexpr (diag) {
            return sliderAttackBB.at(arrBishopBase[sq] + _pext_u64(occ, arrMask<diag>[sq]));
        } else {
            return sliderAttackBB.at(arrRookBase[sq] + _pext_u64(occ, arrMask<diag>[sq]));
        }
    }

    template<bool diag>
    void handle_pext(int sq, int& offset) {
        BB fullMask = slideMaskSlow<diag>(0, sq);
        if(!hasBitAt(fileA, sq)) fullMask &= ~fileA;
        if(!hasBitAt(fileH, sq)) fullMask &= ~fileH;
        if(!hasBitAt(rank1, sq)) fullMask &= ~rank1;
        if(!hasBitAt(rank8, sq)) fullMask &= ~rank8;

        arrMask<diag>.at(sq) = fullMask;
        int numOccupancies = 1 << bitCount(fullMask);
        for (int cand = 0; cand < numOccupancies; ++cand) {
            BB occ = 0, mask = fullMask, extractor = 1;
            Bitloop(mask) {
                if(cand & extractor) {
                    occ |= isolateLowestBit(mask);
                }
                extractor <<= 1;
            }

            sliderAttackBB.at(offset + cand) = slideMaskSlow<diag>(occ, sq);
        }
        offset += numOccupancies;
    }

    void init_pext() {
        int offset = 0;
        for (int sq = 0; sq < 64; ++sq) {
            arrRookBase.at(sq) = offset;
            handle_pext<false>(sq, offset);
            arrBishopBase.at(sq) = offset;
            handle_pext<true>(sq, offset);
        }
        std::cout << offset << std::endl;
        
    }

    void load() {
        if(!loaded) {
            for(int i = 0; i < 64; i++) {
                calculate_lines<true>(i);
                calculate_lines<false>(i);
                addKnightMoves(i);
                addKingMoves(i);
            }
            init_pext();
            loaded = true;
        }
    }

}

#endif //DORY_PIECESTEPS_H
