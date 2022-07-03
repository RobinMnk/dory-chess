//
// Created by Robin on 29.06.2022.
//

#include "chess.h"

#define CHECK_BIT(var, pos) (var & (1 << pos))
#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H

struct State {
    constexpr State(bool white, BB ep, bool wcs, bool bcs, bool wcl, bool bcl) :
        whiteToMove{white}, enPassantField{ep}, wCastleShort{wcs}, bCastleShort{bcs},
        wCastleLong{wcl}, bCastleLong{bcl} {}

    const bool whiteToMove;
    const BB enPassantField;
    const bool wCastleShort, bCastleShort;
    const bool wCastleLong, bCastleLong;

    [[nodiscard]] constexpr bool canCastleShort() const {
        return whiteToMove ? wCastleShort : bCastleShort;
    }

    [[nodiscard]] constexpr bool canCastleLong() const {
        return whiteToMove ? wCastleLong : bCastleLong;
    }
};

constexpr State STARTSTATE = State(true, 0, true, true, true, true);

class Board {
public:
    const BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{0}, bQueens{0}, wKing{0}, bKing{0};

    Board() = default;
    constexpr Board(BB wP, BB bP, BB wN, BB bN, BB wB, BB bB, BB wR, BB bR, BB wQ, BB bQ, BB wK, BB bK) :
            wPawns{wP}, bPawns{bP}, wKnights{wN}, bKnights{bN}, wBishops{wB}, bBishops{bB},
            wRooks{wR}, bRooks{bR}, wQueens{wQ}, bQueens{bQ}, wKing{wK}, bKing{bK} {}

    template<bool whiteToMove> [[nodiscard]] constexpr BB pawns() const     { return whiteToMove ? wPawns : bPawns; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB knights() const   { return whiteToMove ? wKnights : bKnights; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB bishops() const   { return whiteToMove ? wBishops : bBishops; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB rooks() const     { return whiteToMove ? wRooks : bRooks; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB queens() const    { return whiteToMove ? wQueens : bQueens; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB king() const      { return whiteToMove ? wKing : bKing; }

    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyPawns() const     { return whiteToMove ? bPawns : wPawns; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyKnights() const   { return whiteToMove ? bKnights : wKnights; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyBishops() const   { return whiteToMove ? bBishops : wBishops; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyRooks() const     { return whiteToMove ? bRooks : wRooks; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyQueens() const    { return whiteToMove ? bQueens : wQueens; }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyKing() const      { return whiteToMove ? bKing : wKing; }

    template<bool whiteToMove>
    [[nodiscard]] constexpr int kingSquare() const {
        return whiteToMove ? singleBitOf(wKing) : singleBitOf(bKing);
    }

    [[nodiscard]] constexpr BB occ() const {
        return wPawns | wKnights | wBishops | wRooks | wQueens | wKing
                 | bPawns | bKnights | bBishops | bRooks | bQueens | bKing;
    }

    [[nodiscard]] constexpr BB free() const {
        return ~occ();
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr BB allPieces() const {
        return whiteToMove ? wPawns | wKnights | wBishops | wRooks | wQueens | wKing
                       : bPawns | bKnights | bBishops | bRooks | bQueens | bKing;
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr BB myPieces() const {
        return allPieces<whiteToMove>();
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr BB enemyPieces() const {
        return allPieces<!whiteToMove>();
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr BB enemyOrEmpty() const {
        return ~allPieces<whiteToMove>();
    }

    template<bool whiteToMove, bool diag>
    [[nodiscard]] constexpr BB enemySliders() const {
        return whiteToMove ? bQueens | (diag ? bBishops : bRooks) : wQueens | (diag ? wBishops : wRooks);
    }


};


constexpr Board STARTBOARD = Board(rank2, rank7, 0x42, 0x42ull << 7*8, 0x24, 0x24ull << 7*8, 0x81, 0x81ull << 7*8, 0x8, 0x8ull << 7*8, 0x10, 0x10ull << 7*8);



// ---------- BITBOARD UTILS ----------

constexpr BB FULL_BB = 0xffffffffffffffff;



template<State state>
constexpr BB pawnPush(BB pawns) {
    return state.whiteToMove ? pawns << 8 : pawns >> 8;
}

#endif //CHESSENGINE_BOARD_H

