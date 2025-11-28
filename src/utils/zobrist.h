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
    static std::array<BB, 64> EP_SQUARE{};
    static BB black_to_move_bitstring;
    static Utils::Random random;

    void init(size_t seed=0) {
        random.setSeed(seed);
        for(int sq = 0; sq < 64; ++sq) {
            for(int p = 0; p < 12; ++p) {
                BITSTRINGS[sq][p] = random.randomBitstring();
            }
            EP_SQUARE[sq] = random.randomBitstring();
        }
        black_to_move_bitstring = random.randomBitstring();
    }

    void hashPieceBB(BB pieces, int pieceIx, uint64_t& hash) {
        Bitloop(pieces) {
            int ix = firstBitOf(pieces);
            hash ^= BITSTRINGS[ix][pieceIx];
        }
    }

    template<bool whiteToMove>
    BB hash(const Board& board) {
        BB h{0};
        if constexpr (!whiteToMove)
            h ^= black_to_move_bitstring;

        h ^= EP_SQUARE[board.enPassantSq];
        h ^= BITSTRINGS [board.wKingSq][PIECE_King];
        h ^= BITSTRINGS [board.bKingSq][PIECE_King + PIECE_None];

        hashPieceBB(board.wPawns, PIECE_Pawn, h);
        hashPieceBB(board.wKnights, PIECE_Knight, h);
        hashPieceBB(board.wBishops, PIECE_Bishop, h);
        hashPieceBB(board.wRooks, PIECE_Rook, h);
        hashPieceBB(board.wQueens, PIECE_Queen, h);

        hashPieceBB(board.bPawns, PIECE_Pawn + PIECE_None, h);
        hashPieceBB(board.bKnights, PIECE_Knight + PIECE_None, h);
        hashPieceBB(board.bBishops, PIECE_Bishop + PIECE_None, h);
        hashPieceBB(board.bRooks, PIECE_Rook + PIECE_None, h);
        hashPieceBB(board.bQueens, PIECE_Queen + PIECE_None, h);

        return h;
    }

} // namespace Dory::Zobrist

#endif //DORY_ZOBRIST_H
