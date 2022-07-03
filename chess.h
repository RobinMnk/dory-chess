//
// Created by Robin on 29.06.2022.
//

#include <cstdint>
#include <immintrin.h>
#include <bit>

#ifndef CHESSENGINE_CHESS_H
#define CHESSENGINE_CHESS_H
#define SquareOf(X) _tzcnt_u64(X)
#define Bitloop(X) for(;X; X = _blsr_u64(X))

using BB = uint64_t;
using square = uint64_t;

// ---------- PIECES ----------

namespace Piece {
    const uint8_t King = 1;
    const uint8_t Queen = 2;
    const uint8_t Rook = 3;
    const uint8_t Bishop = 4;
    const uint8_t Knight = 5;
    const uint8_t Pawn = 6;
}

namespace MoveFlag {
    const uint8_t Silent = 0;
    const uint8_t RemoveShortCastling = 1;
    const uint8_t RemoveLongCastling = 2;
    const uint8_t RemoveAllCastling = 3;
}



// ---------- BOARD GEOMETRY ----------

static constexpr BB file1 = 0x8080808080808080;
static constexpr BB file2 = 0x4040404040404040;
static constexpr BB file3 = 0x2020202020202020;
static constexpr BB file4 = 0x1010101010101010;
static constexpr BB file5 = 0x0808080808080808;
static constexpr BB file6 = 0x0404040404040404;
static constexpr BB file7 = 0x0202020202020202;
static constexpr BB file8 = 0x0101010101010101;

static constexpr BB rank1 = 0xff;
static constexpr BB rank2 = 0xff00;
static constexpr BB rank3 = 0xff0000;
static constexpr BB rank4 = 0xff000000;
static constexpr BB rank5 = 0xff00000000;
static constexpr BB rank6 = 0xff0000000000;
static constexpr BB rank7 = 0xff000000000000;
static constexpr BB rank8 = 0xff00000000000000;

static constexpr int rankOf(int square) {
    return square / 8;
}

static constexpr int fileOf(int square) {
    return square % 8;
}

template<bool whiteToMove>
static constexpr int epRankNr() {
    return whiteToMove ? 4 : 3;
}

template<bool white>
static constexpr int startingPosKingsideRook() {
    return white ? 7 : 63;
}

template<bool white>
static constexpr int startingPosQueensideRook() {
    return white ? 0 : 56;
}


// ---------- BIT OPERATIONS ----------

static constexpr BB newMask(int index) {
    return 1ull << index;
}

static constexpr void setBit(BB& board, int index) {
    board |= newMask(index);
}

static constexpr BB withBit(BB board, int index) {
    return board | newMask(index);
}

static constexpr void deleteBitAt(BB& board, int index) {
    board &= ~(1L << index);
}

static constexpr bool hasBitAt(BB board, int index) {
    return (board & (1L << index)) != 0;
}

static constexpr int bitCount(BB number) {
    return std::popcount(number);
}

static constexpr int firstBitOf(BB number) {
    return __builtin_ctz(number);
}

static constexpr int lastBitOf(BB number) {
    return 63 - firstBitOf(number);
}

static constexpr int singleBitOf(BB number) {
    return firstBitOf(number);
}

#endif //CHESSENGINE_CHESS_H
