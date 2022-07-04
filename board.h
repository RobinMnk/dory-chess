//
// Created by Robin on 29.06.2022.
//

#include "chess.h"

#define CHECK_BIT(var, pos) (var & (1 << pos))
#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H

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


template<bool white>
static constexpr BB startingKingsideRook() {
    return white ? newMask(7) : newMask(63);
}

template<bool white>
static constexpr BB startingQueensideRook() {
    return white ? newMask(0) : newMask(56);
}

template<bool isWhite>
constexpr BB castleShortMask() {
    return isWhite ? 0b111ull << singleBitOf(STARTBOARD.wKing) : 0b111ull << singleBitOf(STARTBOARD.bKing);
}

template<bool isWhite>
constexpr BB castleLongMask() {
    return isWhite ? 0b111ull << (singleBitOf(STARTBOARD.wKing)-2) : 0b111ull << (singleBitOf(STARTBOARD.bKing)-2);
}

template<bool isWhite>
constexpr BB castleShortRookMove() {
    return isWhite ? 0b101ull << (singleBitOf(STARTBOARD.wKing) + 1) : 0b101ull << (singleBitOf(STARTBOARD.bKing) + 1);
}

template<bool isWhite>
constexpr BB castleLongRookMove() {
    return isWhite ? 0b1001ull : 0b1001ull << (singleBitOf(STARTBOARD.bKing) - 4);
}

template<bool whiteToMove> constexpr BB forward(BB bb) {
    return whiteToMove ? bb << 8 : bb >> 8;
}
template<bool whiteToMove> constexpr BB backward(BB bb) {
    return whiteToMove ?  bb >> 8 : bb << 8;
}
template<bool whiteToMove> constexpr BB forward2(BB bb) {
    return whiteToMove ? bb << 16 : bb >> 16;
}
template<bool whiteToMove> constexpr BB backward2(BB bb) {
    return whiteToMove ?  bb >> 16 : bb << 16;
}
template<bool whiteToMove> constexpr BB pawnAtkLeft(BB bb) {
    return whiteToMove ? bb << 7 : bb >> 7;
}
template<bool whiteToMove> constexpr BB pawnAtkRight(BB bb) {
    return whiteToMove ? bb << 9 : bb >> 9;
}
template<bool whiteToMove> constexpr BB pawnInvAtkLeft(BB bb) {
    return whiteToMove ? bb >> 7 : bb << 7;
}
template<bool whiteToMove> constexpr BB pawnInvAtkRight(BB bb) {
    return whiteToMove ? bb >> 9 : bb << 9;
}
template<bool whiteToMove> constexpr BB pawnCanGoLeft() {
    return whiteToMove ? ~file1 : ~file8;
}
template<bool whiteToMove> constexpr BB pawnCanGoRight() {
    return whiteToMove ? ~file8 : ~file1;
}
template<bool whiteToMove> constexpr BB pawnOnLastRow() {
    return whiteToMove ? rank7 : rank2;
}
template<bool whiteToMove> constexpr BB firstRank() {
    return whiteToMove ? rank2 : rank7;
}

struct State {
    constexpr State(bool white, BB ep, bool wcs, bool wcl, bool bcs, bool bcl) :
            whiteToMove{white}, enPassantField{ep}, wCastleShort{wcs}, wCastleLong{wcl},
            bCastleShort{bcs}, bCastleLong{bcl} {}

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

constexpr State STARTSTATE = State(true, 0ull, true, true, true, true);

template<State state>
constexpr State next(Move move) {
    constexpr bool white = state.whiteToMove;

    constexpr BB epField = move.flags == MoveFlag::PawnDoublePush ? forward<white>(move.from) : 0ull;

    if(move.flags == MoveFlag::RemoveShortCastling) {
        if(white) return {!white, epField, false, state.wCastleLong, state.bCastleShort, state.bCastleLong};
        else return {!white, epField, state.wCastleShort, state.wCastleLong, false, state.bCastleLong};
    }

    if(move.flags == MoveFlag::RemoveLongCastling) {
        if(white) return {!white, epField, state.wCastleShort, false, state.bCastleShort, state.bCastleLong};
        else return {!white, epField, state.wCastleShort, state.wCastleLong, state.bCastleShort, false};
    }

    if(move.flags == MoveFlag::RemoveAllCastling || move.flags == MoveFlag::ShortCastling || move.flags == MoveFlag::LongCastling) {
        if(white) return {!white, epField, false, false, state.bCastleShort, state.bCastleLong};
        else return {!white, epField, state.wCastleShort, state.wCastleLong, false, false};
    }

    return {!white, epField, state.wCastleShort, state.wCastleLong, state.bCastleShort, state.bCastleLong};
}

template<Board board, State state>
constexpr Board next(Move move) {
    constexpr bool whiteMoved = state.whiteToMove;

    // Promotions
    if(move.flags == MoveFlag::PromoteQueen) {
        if(whiteMoved) return {board.wPawns & ~move.to, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens | move.to, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns & ~move.to, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens | move.to, board.wKing, board.bKing};
    }
    if(move.flags == MoveFlag::PromoteRook) {
        if(whiteMoved) return {board.wPawns & ~move.to, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks | move.to, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns & ~move.to, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks | move.to, board.wQueens, board.bQueens, board.wKing, board.bKing};
    }
    if(move.flags == MoveFlag::PromoteBishop) {
        if(whiteMoved) return {board.wPawns & ~move.to, board.bPawns, board.wKnights, board.bKnights, board.wBishops | move.to, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns & ~move.to, board.wKnights, board.bKnights, board.wBishops, board.bBishops | move.to, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
    }
    if(move.flags == MoveFlag::PromoteKnight) {
        if(whiteMoved) return {board.wPawns & ~move.to, board.bPawns, board.wKnights | move.to, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns & ~move.to, board.wKnights, board.bKnights | move.to, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
    }

    BB change = move.from | move.to;

    //Castles
    if(move.flags == MoveFlag::ShortCastling) {
        if(whiteMoved) return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks ^ castleShortRookMove<whiteMoved>(), board.bRooks, board.wQueens, board.bQueens, board.wKing ^ change, board.bKing};
        return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks ^ castleShortRookMove<whiteMoved>(), board.wQueens, board.bQueens, board.wKing, board.bKing ^ change};
    }
    if(move.flags == MoveFlag::ShortCastling) {
        if(whiteMoved) return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks ^ castleLongRookMove<whiteMoved>(), board.bRooks, board.wQueens, board.bQueens, board.wKing ^ change, board.bKing};
        return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks ^ castleLongRookMove<whiteMoved>(), board.wQueens, board.bQueens, board.wKing, board.bKing ^ change};
    }

    // Silent Moves
    if(move.piece == Piece::Pawn) {
        BB epMask = move.flags == MoveFlag::EnPassantCapture ? ~backward<whiteMoved>(state.enPassantField) : FULL_BB;
        if(whiteMoved) return {board.wPawns ^ change, board.bPawns & epMask, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns & epMask, board.bPawns ^ change, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
    }
    if(move.piece == Piece::Knight) {
        if(whiteMoved) return {board.wPawns, board.bPawns, board.wKnights ^ change, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns, board.wKnights, board.bKnights ^ change, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
    }
    if(move.piece == Piece::Bishop) {
        if(whiteMoved) return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops ^ change, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops ^ change, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
    }
    if(move.piece == Piece::Rook) {
        if(whiteMoved) return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks ^ change, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks ^ change, board.wQueens, board.bQueens, board.wKing, board.bKing};
    }
    if(move.piece == Piece::Queen) {
        if(whiteMoved) return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens ^ change, board.bQueens, board.wKing, board.bKing};
        return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens ^ change, board.wKing, board.bKing};
    }
    if(move.piece == Piece::King) {
        if(whiteMoved) return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing ^ change, board.bKing};
        return {board.wPawns, board.bPawns, board.wKnights, board.bKnights, board.wBishops, board.bBishops, board.wRooks, board.bRooks, board.wQueens, board.bQueens, board.wKing, board.bKing ^ change};
    }
}

#endif //CHESSENGINE_BOARD_H

