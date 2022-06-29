//
// Created by Robin on 29.06.2022.
//

#include "chess.h"

#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H

struct State {
    constexpr State(BB ep, bool white, bool wcs, bool bcs, bool wcl, bool bcl) :
        enPassantField{ep}, whiteToMove{white}, wCastleShort{wcs}, bCastleShort{bcs},
        wCastleLong{wcl}, bCastleLong{bcl} {}

    const bool whiteToMove;
    const BB enPassantField;
    const bool wCastleShort, bCastleShort;
    const bool wCastleLong, bCastleLong;
};

constexpr State STARTSTATE = State(0, true, true, true, true, true);

class Board {
public:
    Board() = default;
    constexpr Board(BB wP, BB bP, BB wN, BB bN, BB wB, BB bB, BB wR, BB bR, BB wQ, BB bQ, BB wK, BB bK) :
            wPawns{wP}, bPawns{bP}, wKnights{wN}, bKnights{bN}, wBishops{wB}, bBishops{bB},
            wRooks{wR}, bRooks{bR}, wQueens{wQ}, bQueens{bQ}, wKing{wK}, bKing{bK} {}

    const BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{0}, bQueens{0}, wKing{0}, bKing{0};
};


constexpr Board STARTBOARD = Board(rank2, rank7, 0x42, 0x42ull << 7*8, 0x24, 0x24ull << 7*8, 0x81, 0x81ull << 7*8, 0x8, 0x8ull << 7*8, 0x10, 0x10ull << 7*8);


template<State state>
constexpr BB pawnPush(BB pawns) {
    return state.whiteToMove ? pawns << 8 : pawns >> 8;
}

#endif //CHESSENGINE_BOARD_H

