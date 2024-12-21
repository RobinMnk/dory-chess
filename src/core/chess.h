//
// Created by Robin on 29.06.2022.
//

#ifndef DORY_CHESS_H
#define DORY_CHESS_H

#include <cstdint>
#include <bit>
#include <immintrin.h>

namespace Dory {

    #define Bitloop(X) for(;X; X = _blsr_u64(X))

    using BB = uint64_t;
    using square = uint64_t;

    const int INF = 2140000000;

// ---------- PIECES ----------

    using Piece_t = uint8_t;
    const Piece_t PIECE_Queen = 0;
    const Piece_t PIECE_Rook = 1;
    const Piece_t PIECE_Bishop = 2;
    const Piece_t PIECE_Knight = 3;
    const Piece_t PIECE_Pawn = 4;
    const Piece_t PIECE_King = 5;

    using Flag_t = uint8_t;
    const Flag_t MOVEFLAG_Silent = 0;
    const Flag_t MOVEFLAG_RemoveShortCastling = 1;
    const Flag_t MOVEFLAG_RemoveLongCastling = 2;
    const Flag_t MOVEFLAG_RemoveAllCastling = 3;
    const Flag_t MOVEFLAG_PawnDoublePush = 4;
    const Flag_t MOVEFLAG_EnPassantCapture = 5;
    const Flag_t MOVEFLAG_PromoteQueen = 6;
    const Flag_t MOVEFLAG_PromoteRook = 7;
    const Flag_t MOVEFLAG_PromoteBishop = 8;
    const Flag_t MOVEFLAG_PromoteKnight = 9;
    const Flag_t MOVEFLAG_ShortCastling = 10;
    const Flag_t MOVEFLAG_LongCastling = 11;

// ---------- BOARD GEOMETRY ----------

    static constexpr BB fileH = 0x8080808080808080;
    static constexpr BB fileG = 0x4040404040404040;
    static constexpr BB fileF = 0x2020202020202020;
    static constexpr BB fileE = 0x1010101010101010;
    static constexpr BB fileD = 0x0808080808080808;
    static constexpr BB fileC = 0x0404040404040404;
    static constexpr BB fileB = 0x0202020202020202;
    static constexpr BB fileA = 0x0101010101010101;

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
    static constexpr BB epRank() {
        if constexpr (whiteToMove) return rank5;
        else return rank4;
    }


// ---------- BIT OPERATIONS ----------

    static inline constexpr BB newMask(int index) {
        return 1ull << index;
    }

    static inline constexpr void setBit(BB &board, int index) {
        board |= newMask(index);
    }

    static inline constexpr BB withBit(BB board, int index) {
        return board | newMask(index);
    }

    static constexpr void deleteBitAt(BB &board, int index) {
        board &= ~(1ull << index);
    }

    static inline constexpr bool hasBitAt(BB board, int index) {
        return (board & (1ull << index)) != 0;
    }

    static inline constexpr int bitCount(BB number) {
        return std::popcount(number);
    }

    static inline constexpr int firstBitOf(BB number) {
        return __builtin_ctzll(number);
    }

    static inline constexpr int lastBitOf(BB number) {
        return 63 - __builtin_clzll(number);
    }

    static inline constexpr int singleBitOf(BB number) {
        return firstBitOf(number);
    }

    static inline BB isolateLowestBit(BB number) {
        // calculates number & -number;
        return _blsi_u64(number);
    }

    // ------------- PAWN MOVES -------------

    template<bool whiteToMove>
    constexpr BB forward(BB bb) {
        if constexpr (whiteToMove) return bb << 8;
        else return bb >> 8;
    }

    template<bool whiteToMove>
    constexpr BB backward(BB bb) {
        if constexpr (whiteToMove) return bb >> 8;
        else return bb << 8;
    }

    template<bool whiteToMove>
    constexpr BB forward2(BB bb) {
        if constexpr (whiteToMove) return bb << 16;
        else return bb >> 16;
    }

    template<bool whiteToMove>
    constexpr BB backward2(BB bb) {
        if constexpr (whiteToMove) return bb >> 16;
        else return bb << 16;
    }

    template<bool whiteToMove>
    constexpr BB pawnAtkLeft(BB bb) {
        if constexpr (whiteToMove) return bb << 7;
        else return bb >> 7;
    }

    template<bool whiteToMove>
    constexpr BB pawnAtkRight(BB bb) {
        if constexpr (whiteToMove) return bb << 9;
        else return bb >> 9;
    }

    template<bool whiteToMove>
    constexpr BB pawnInvAtkLeft(BB bb) {
        if constexpr (whiteToMove) return bb >> 7;
        else return bb << 7;
    }

    template<bool whiteToMove>
    constexpr BB pawnInvAtkRight(BB bb) {
        if constexpr (whiteToMove) return bb >> 9;
        else return bb << 9;
    }

    template<bool whiteToMove>
    constexpr BB pawnCanGoLeft() {
        if constexpr (whiteToMove) return ~fileA;
        else return ~fileH;
    }

    template<bool whiteToMove>
    constexpr BB pawnCanGoRight() {
        if constexpr (whiteToMove) return ~fileH;
        else return ~fileA;
    }

    template<bool whiteToMove>
    constexpr BB pawnOnLastRow() {
        if constexpr (whiteToMove) return rank7;
        else return rank2;
    }

    template<bool whiteToMove>
    constexpr BB firstPawnRank() {
        if constexpr (whiteToMove) return rank2;
        else return rank7;
    }

    template<bool whiteToMove>
    inline constexpr BB backRank() {
        if constexpr (whiteToMove) return rank1;
        else return rank8;
    }


    // ---------------  MOVES ----------------

    struct Move {
        // Optimization potential: piece and flags can be stored in just one byte (3 bits piece, 4 bits flags)
        uint8_t fromIndex{0}, toIndex{0}, piece{0}, flags{0};

        constexpr Move(uint8_t fromIx, uint8_t toIx, Piece_t pc, Flag_t fl)
            : fromIndex{fromIx}, toIndex{toIx}, piece{pc}, flags{fl} {}

        constexpr Move() = default;

        [[nodiscard]] BB from() const {
            return newMask(fromIndex);
        }

        [[nodiscard]] BB to() const {
            return newMask(toIndex);
        }

        bool operator==(const Move &other) const = default;

        [[nodiscard]] bool is(BB fromBB, BB toBB, Piece_t pc, Flag_t fl) const {
            return pc == piece && fl == flags && fromIndex == singleBitOf(fromBB) && toIndex == singleBitOf(toBB);
        }

        template<Piece_t pc, Flag_t fl>
        [[nodiscard]] bool is(BB fromBB, BB toBB) const {
            return pc == piece && fl == flags && fromIndex == singleBitOf(fromBB) && toIndex == singleBitOf(toBB);
        }
    };

    constexpr Move createMoveFromBB(BB from, BB to, Piece_t pc, Flag_t fl) {
        return {
                static_cast<uint8_t>(singleBitOf(from)),
                static_cast<uint8_t>(singleBitOf(to)),
                pc, fl
        };
    }

    constexpr static const Move NULLMOVE{0, 0, 0, 0};

} // namespace Dory

#endif //DORY_CHESS_H
