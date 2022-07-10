//
// Created by Robin on 29.06.2022.
//

#include "chess.h"

#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H


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

    template<Flag_t flag = MoveFlag::Silent, BB epField = 0ull>
    [[nodiscard]] constexpr State next() const {
        if(flag == MoveFlag::RemoveShortCastling) {
            if(whiteToMove) return {!whiteToMove, 0ull, false, wCastleLong, bCastleShort, bCastleLong};
            else return {!whiteToMove, 0ull, wCastleShort, wCastleLong, false, bCastleLong};
        }

        if(flag == MoveFlag::RemoveLongCastling) {
            if(whiteToMove) return {!whiteToMove, 0ull, wCastleShort, false, bCastleShort, bCastleLong};
            else return {!whiteToMove, 0ull, wCastleShort, wCastleLong, bCastleShort, false};
        }

        if(flag == MoveFlag::RemoveAllCastling || flag == MoveFlag::ShortCastling || flag == MoveFlag::LongCastling) {
            if(whiteToMove) return {!whiteToMove, 0ull, false, false, bCastleShort, bCastleLong};
            else return {!whiteToMove, 0ull, wCastleShort, wCastleLong, false, false};
        }

        if(flag == MoveFlag::PawnDoublePush) {
            return {!whiteToMove, epField, wCastleShort, wCastleLong, bCastleShort, bCastleLong};
        }

        return {!whiteToMove, 0ull, wCastleShort, wCastleLong, bCastleShort, bCastleLong};
    }
};

constexpr State STARTSTATE = State(true, 0ull, true, true, true, true);

// forward definitions of functions needed within Board
template<bool>
constexpr BB castleShortRookMove();

template<bool>
constexpr BB castleLongRookMove();

template<bool whiteToMove>
constexpr BB backward(BB bb);

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
        return ~myPieces<whiteToMove>();
    }

    template<bool whiteToMove, bool diag>
    [[nodiscard]] constexpr BB enemySliders() const {
        return whiteToMove ? bQueens | (diag ? bBishops : bRooks) : wQueens | (diag ? wBishops : wRooks);
    }

    template<State state, Flag_t flags>
    [[nodiscard]] constexpr Board next(Piece_t piece, BB from, BB to) const {
        constexpr bool whiteMoved = state.whiteToMove;

        // Promotions
        if(flags == MoveFlag::PromoteQueen) {
            if(whiteMoved) return {wPawns & ~to, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens | to, bQueens, wKing, bKing};
            return {wPawns, bPawns & ~to, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens | to, wKing, bKing};
        }
        if(flags == MoveFlag::PromoteRook) {
            if(whiteMoved) return {wPawns & ~to, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks | to, bRooks, wQueens, bQueens, wKing, bKing};
            return {wPawns, bPawns & ~to, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks | to, wQueens, bQueens, wKing, bKing};
        }
        if(flags == MoveFlag::PromoteBishop) {
            if(whiteMoved) return {wPawns & ~to, bPawns, wKnights, bKnights, wBishops | to, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
            return {wPawns, bPawns & ~to, wKnights, bKnights, wBishops, bBishops | to, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
        }
        if(flags == MoveFlag::PromoteKnight) {
            if(whiteMoved) return {wPawns & ~to, bPawns, wKnights | to, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
            return {wPawns, bPawns & ~to, wKnights, bKnights | to, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
        }

        BB change = from | to;

        //Castles
        if(flags == MoveFlag::ShortCastling) {
            if(whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleShortRookMove<whiteMoved>(), bRooks, wQueens, bQueens, wKing ^ change, bKing};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleShortRookMove<whiteMoved>(), wQueens, bQueens, wKing, bKing ^ change};
        }
        if(flags == MoveFlag::LongCastling) {
            if(whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleLongRookMove<whiteMoved>(), bRooks, wQueens, bQueens, wKing ^ change, bKing};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleLongRookMove<whiteMoved>(), wQueens, bQueens, wKing, bKing ^ change};
        }

        // Silent Moves
        if(piece == Piece::Pawn) {
            BB epMask = flags == MoveFlag::EnPassantCapture ? ~backward<whiteMoved>(state.enPassantField) : FULL_BB;
            if(whiteMoved) return {wPawns ^ change, bPawns & epMask, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
            return {wPawns & epMask, bPawns ^ change, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
        }
        if(piece == Piece::Knight) {
            if(whiteMoved) return {wPawns, bPawns, wKnights ^ change, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
            return {wPawns, bPawns, wKnights, bKnights ^ change, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
        }
        if(piece == Piece::Bishop) {
            if(whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops ^ change, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops ^ change, wRooks, bRooks, wQueens, bQueens, wKing, bKing};
        }
        if(piece == Piece::Rook) {
            if(whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ change, bRooks, wQueens, bQueens, wKing, bKing};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ change, wQueens, bQueens, wKing, bKing};
        }
        if(piece == Piece::Queen) {
            if(whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens ^ change, bQueens, wKing, bKing};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens ^ change, wKing, bKing};
        }
        if(piece == Piece::King) {
            if(whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing ^ change, bKing};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing ^ change};
        }
        throw std::exception();
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

#endif //CHESSENGINE_BOARD_H

