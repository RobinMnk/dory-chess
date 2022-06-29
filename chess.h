//
// Created by Robin on 29.06.2022.
//

#include <cstdint>
#include <immintrin.h>

#ifndef CHESSENGINE_CHESS_H
#define CHESSENGINE_CHESS_H
#define SquareOf(X) _tzcnt_u64(X)
#define Bitloop(X) for(;X; X = _blsr_u64(X))

using BB = uint64_t;
using square = uint64_t;

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

static constexpr int rank(int square) {
    return square / 8;
}

static constexpr int file(int square) {
    return square % 8;
}


#endif //CHESSENGINE_CHESS_H
