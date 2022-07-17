//
// Created by Robin on 29.06.2022.
//

#include <cstdint>
#include <immintrin.h>
#include <bit>

#ifndef CHESSENGINE_CHESS_H
#define CHESSENGINE_CHESS_H
#define Bitloop(X) for(;X; X = _blsr_u64(X))

using BB = uint64_t;
using square = uint64_t;

// ---------- PIECES ----------

using Piece_t = uint8_t;
namespace Piece {
    const Piece_t King = 1;
    const Piece_t Queen = 2;
    const Piece_t Rook = 3;
    const Piece_t Bishop = 4;
    const Piece_t Knight = 5;
    const Piece_t Pawn = 6;
}

using Flag_t = uint8_t;
namespace MoveFlag {
    const Flag_t Silent = 0;
    const Flag_t RemoveShortCastling = 1;
    const Flag_t RemoveLongCastling = 2;
    const Flag_t RemoveAllCastling = 3;
    const Flag_t PawnDoublePush = 4;
    const Flag_t EnPassantCapture = 5;
    const Flag_t PromoteQueen = 6;
    const Flag_t PromoteRook = 7;
    const Flag_t PromoteBishop = 8;
    const Flag_t PromoteKnight = 9;
    const Flag_t ShortCastling = 10;
    const Flag_t LongCastling = 11;
}

struct Move {
    BB from{0}, to{0};
    uint8_t piece{0}, flags{0};
};


// ---------- BOARD GEOMETRY ----------

static constexpr BB file8 = 0x8080808080808080;
static constexpr BB file7 = 0x4040404040404040;
static constexpr BB file6 = 0x2020202020202020;
static constexpr BB file5 = 0x1010101010101010;
static constexpr BB file4 = 0x0808080808080808;
static constexpr BB file3 = 0x0404040404040404;
static constexpr BB file2 = 0x0202020202020202;
static constexpr BB file1 = 0x0101010101010101;

static constexpr BB rank1 = 0xff;
static constexpr BB rank2 = 0xff00;
static constexpr BB rank3 = 0xff0000;
static constexpr BB rank4 = 0xff000000;
static constexpr BB rank5 = 0xff00000000;
static constexpr BB rank6 = 0xff0000000000;
static constexpr BB rank7 = 0xff000000000000;
static constexpr BB rank8 = 0xff00000000000000;

// ---------- BITBOARD UTILS ----------

constexpr BB FULL_BB = 0xffffffffffffffff;

static constexpr int rankOf(int square) {
    return square / 8;
}

static constexpr int fileOf(int square) {
    return square % 8;
}

template<bool whiteToMove>
static constexpr int epRankNr() {
    if constexpr (whiteToMove) return 4;
    else return 3;
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
    board &= ~(1ull << index);
}

static constexpr bool hasBitAt(BB board, int index) {
    return (board & (1ull << index)) != 0;
}

static constexpr int bitCount(BB number) {
    return std::popcount(number);
}

static constexpr int firstBitOf(BB number) {
    return __builtin_clzll(number);
}

static constexpr int lastBitOf(BB number) {
    return __builtin_ctzll(number);
}

static constexpr int singleBitOf(BB number) {
    return lastBitOf(number);
}

// ------------- PAWN MOVES -------------

template<bool whiteToMove> constexpr BB forward(BB bb) {
    if constexpr (whiteToMove) return bb << 8;
    else return bb >> 8;
}
template<bool whiteToMove> constexpr BB backward(BB bb) {
    if constexpr (whiteToMove) return bb >> 8;
    else return bb << 8;
}
template<bool whiteToMove> constexpr BB forward2(BB bb) {
    if constexpr (whiteToMove) return bb << 16;
    else return bb >> 16;
}
template<bool whiteToMove> constexpr BB backward2(BB bb) {
    if constexpr (whiteToMove) return bb >> 16;
    else return bb << 16;
}
template<bool whiteToMove> constexpr BB pawnAtkLeft(BB bb) {
    if constexpr (whiteToMove) return bb << 7;
    else return bb >> 7;
}
template<bool whiteToMove> constexpr BB pawnAtkRight(BB bb) {
    if constexpr (whiteToMove) return bb << 9;
    else return bb >> 9;
}
template<bool whiteToMove> constexpr BB pawnInvAtkLeft(BB bb) {
    if constexpr (whiteToMove) return bb >> 7;
    else return bb << 7;
}
template<bool whiteToMove> constexpr BB pawnInvAtkRight(BB bb) {
    if constexpr (whiteToMove) return bb >> 9;
    else return bb << 9;
}
template<bool whiteToMove> constexpr BB pawnCanGoLeft() {
    if constexpr (whiteToMove) return ~file1;
    else return ~file8;
}
template<bool whiteToMove> constexpr BB pawnCanGoRight() {
    if constexpr (whiteToMove) return ~file8;
    else return ~file1;
}
template<bool whiteToMove> constexpr BB pawnOnLastRow() {
    if constexpr (whiteToMove) return rank7;
    else return rank2;
}
template<bool whiteToMove> constexpr BB firstRank() {
    if constexpr (whiteToMove) return rank2;
    else return rank7;
}

#endif //CHESSENGINE_CHESS_H
