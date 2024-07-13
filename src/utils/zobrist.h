//
// Created by Robin on 29.08.2023.
//

#ifndef DORY_ZOBRIST_H
#define DORY_ZOBRIST_H

#include <array>
#include "../core/board.h"
#include "random.h"

namespace Dory::Zobrist {

    static std::array<std::array<BB, 12>, 64> BITSTRINGS{};
    static size_t black_to_move_bitstring;
    static Utils::Random random;

    void init(size_t seed=0) {
        random.setSeed(seed);
        for(int sq = 0; sq < 64; ++sq) {
            for(int p = 0; p < 12; ++p) {
                BITSTRINGS[sq][p] = random.randomNumberInRange(0, FULL_BB);
            }
        }
        black_to_move_bitstring = random.randomNumberInRange(0, FULL_BB);
    }

    template<bool whiteToMove>
    size_t hash(const Board& board) {
        size_t h{0};
        if (!whiteToMove)
            h ^= black_to_move_bitstring;

        for(int sq = 0; sq < 64; ++sq) {
            if(hasBitAt(board.wPawns, sq))
                h ^= BITSTRINGS [sq][0];
            else if(hasBitAt(board.wKnights, sq))
                h ^= BITSTRINGS [sq][1];
            else if(hasBitAt(board.wBishops, sq))
                h ^= BITSTRINGS [sq][2];
            else if(hasBitAt(board.wRooks, sq))
                h ^= BITSTRINGS [sq][3];
            else if(hasBitAt(board.wQueens, sq))
                h ^= BITSTRINGS [sq][4];
            else if(sq == board.wKingSq)
                h ^= BITSTRINGS [sq][5];
            else if(hasBitAt(board.bPawns, sq))
                h ^= BITSTRINGS [sq][6];
            else if(hasBitAt(board.bKnights, sq))
                h ^= BITSTRINGS [sq][7];
            else if(hasBitAt(board.bBishops, sq))
                h ^= BITSTRINGS [sq][8];
            else if(hasBitAt(board.bRooks, sq))
                h ^= BITSTRINGS [sq][9];
            else if(hasBitAt(board.bQueens, sq))
                h ^= BITSTRINGS [sq][10];
            else if(sq == board.bKingSq)
                h ^= BITSTRINGS [sq][11];
        }

        return h;
    }

} // namespace Dory::Zobrist

#endif //DORY_ZOBRIST_H
