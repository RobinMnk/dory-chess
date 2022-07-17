//
// Created by Robin on 29.06.2022.
//

#include "chess.h"

#ifndef CHESSENGINE_BOARD_H
#define CHESSENGINE_BOARD_H


struct State {
    constexpr State(bool white, bool wcs, bool wcl, bool bcs, bool bcl) :
            whiteToMove{white}, wCastleShort{wcs}, wCastleLong{wcl},
            bCastleShort{bcs}, bCastleLong{bcl} {}

    const bool whiteToMove;
    const bool wCastleShort, bCastleShort;
    const bool wCastleLong, bCastleLong;
};

constexpr State STARTSTATE = State(true, true, true, true, true);

template<State state>
constexpr bool canCastleShort() {
    if constexpr (state.whiteToMove) return state.wCastleShort;
    else return state.bCastleShort;
}

template<State state>
constexpr bool canCastleLong() {
    if constexpr (state.whiteToMove) return state.wCastleLong;
    else return state.bCastleLong;
}

template<State state, Flag_t flag = MoveFlag::Silent>
consteval State getNextState() {
    if constexpr (flag == MoveFlag::RemoveShortCastling) {
        if constexpr (state.whiteToMove) return {false, false, state.wCastleLong, state.bCastleShort, state.bCastleLong};
        else return {true, state.wCastleShort, state.wCastleLong, false, state.bCastleLong};
    }

    if constexpr (flag == MoveFlag::RemoveLongCastling) {
        if constexpr (state.whiteToMove) return {false, state.wCastleShort, false, state.bCastleShort, state.bCastleLong};
        else return {true, state.wCastleShort, state.wCastleLong, state.bCastleShort, false};
    }

    if constexpr (flag == MoveFlag::RemoveAllCastling || flag == MoveFlag::ShortCastling || flag == MoveFlag::LongCastling) {
        if constexpr (state.whiteToMove) return {false, false, false, state.bCastleShort, state.bCastleLong};
        else return {true, state.wCastleShort, state.wCastleLong, false, false};
    }

    return {!state.whiteToMove, state.wCastleShort, state.wCastleLong, state.bCastleShort, state.bCastleLong};
}


template<bool isWhite>
constexpr BB castleShortRookMove();

template<bool isWhite>
constexpr BB castleLongRookMove();


class Board {
public:
    const BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{0}, bQueens{0}, wKing{0}, bKing{0};
    const BB enPassantField{0};

    Board() = default;
    constexpr Board(BB wP, BB bP, BB wN, BB bN, BB wB, BB bB, BB wR, BB bR, BB wQ, BB bQ, BB wK, BB bK, BB ep) :
            wPawns{wP}, bPawns{bP}, wKnights{wN}, bKnights{bN}, wBishops{wB}, bBishops{bB},
            wRooks{wR}, bRooks{bR}, wQueens{wQ}, bQueens{bQ}, wKing{wK}, bKing{bK}, enPassantField{ep} {}

    template<bool whiteToMove> [[nodiscard]] constexpr BB pawns() const {
        if constexpr (whiteToMove) return wPawns; else return bPawns;
    }
    template<bool whiteToMove> [[nodiscard]] constexpr BB knights() const{
        if constexpr (whiteToMove) return wKnights; else return bKnights;
    }
    template<bool whiteToMove> [[nodiscard]] constexpr BB bishops() const{
        if constexpr (whiteToMove) return wBishops; else return bBishops;
    }
    template<bool whiteToMove> [[nodiscard]] constexpr BB rooks() const {
        if constexpr (whiteToMove) return wRooks; else return bRooks;
    }
    template<bool whiteToMove> [[nodiscard]] constexpr BB queens() const {
        if constexpr (whiteToMove) return wQueens; else return bQueens;
    }
    template<bool whiteToMove> [[nodiscard]] constexpr BB king() const {
        if constexpr (whiteToMove) return wKing; else return bKing;
    }

    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyPawns() const     { return pawns<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyKnights() const   { return knights<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyBishops() const   { return bishops<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyRooks() const     { return rooks<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyQueens() const    { return queens<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyKing() const      { return kingSquare<!whiteToMove>(); }

    template<bool whiteToMove>
    [[nodiscard]] constexpr int kingSquare() const {
        if constexpr (whiteToMove) return singleBitOf(wKing);
        else return singleBitOf(bKing);
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
        if constexpr (whiteToMove) return wPawns | wKnights | wBishops | wRooks | wQueens | wKing;
        else return bPawns | bKnights | bBishops | bRooks | bQueens | bKing;
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
        if constexpr (whiteToMove) return bQueens | (diag ? bBishops : bRooks);
        else return wQueens | (diag ? wBishops : wRooks);
    }

    template<State state, Piece_t piece, Flag_t flags>
    [[nodiscard]] constexpr Board getNextBoard(BB from, BB to) const {
        constexpr bool whiteMoved = state.whiteToMove;

        // Promotions
        if constexpr (flags == MoveFlag::PromoteQueen) {
            if constexpr (whiteMoved) return {wPawns & ~to, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens | to, bQueens, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~to, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens | to, wKing, bKing, 0ull};
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            if constexpr (whiteMoved) return {wPawns & ~to, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks | to, bRooks, wQueens, bQueens, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~to, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks | to, wQueens, bQueens, wKing, bKing, 0ull};
        }
        if constexpr (flags == MoveFlag::PromoteBishop) {
            if constexpr (whiteMoved) return {wPawns & ~to, bPawns, wKnights, bKnights, wBishops | to, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~to, wKnights, bKnights, wBishops, bBishops | to, wRooks, bRooks, wQueens, bQueens, wKing, bKing, 0ull};
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            if constexpr (whiteMoved) return {wPawns & ~to, bPawns, wKnights | to, bKnights, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~to, wKnights, bKnights | to, wBishops, bBishops, wRooks, bRooks, wQueens, bQueens, wKing, bKing, 0ull};
        }

        BB change = from | to;

        //Castles
        if constexpr (flags == MoveFlag::ShortCastling) {
            if constexpr (whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleShortRookMove<whiteMoved>(), bRooks, wQueens, bQueens, wKing ^ change, bKing, 0ull};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleShortRookMove<whiteMoved>(), wQueens, bQueens, wKing, bKing ^ change, 0ull};
        }
        if constexpr (flags == MoveFlag::LongCastling) {
            if constexpr (whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleLongRookMove<whiteMoved>(), bRooks, wQueens, bQueens, wKing ^ change, bKing, 0ull};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleLongRookMove<whiteMoved>(), wQueens, bQueens, wKing, bKing ^ change, 0ull};
        }

        // Silent Moves
        if constexpr (piece == Piece::Pawn) {
            BB epMask = flags == MoveFlag::EnPassantCapture ? ~backward<whiteMoved>(enPassantField) : FULL_BB;
            BB epField = flags == MoveFlag::PawnDoublePush ? forward<whiteMoved>(from) : 0ull;
            if constexpr (whiteMoved) return {wPawns ^ change, bPawns & epMask & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKing, bKing, epField};
            return {wPawns & epMask & ~to, bPawns ^ change, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKing, bKing, epField};
        }
        if constexpr (piece == Piece::Knight) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights ^ change, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights ^ change, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKing, bKing, 0ull};
        }
        if constexpr (piece == Piece::Bishop) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops ^ change, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops ^ change, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKing, bKing, 0ull};
        }
        if constexpr (piece == Piece::Rook) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks ^ change, bRooks & ~to, wQueens, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks ^ change, wQueens & ~to, bQueens, wKing, bKing, 0ull};
        }
        if constexpr (piece == Piece::Queen) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens ^ change, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens ^ change, wKing, bKing, 0ull};
        }
        if constexpr (piece == Piece::King) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKing ^ change, bKing, 0ull};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKing, bKing ^ change, 0ull};
        }
        throw std::exception();
    }
};


constexpr Board STARTBOARD = Board(rank2, rank7, 0x42, 0x42ull << 7*8, 0x24, 0x24ull << 7*8, 0x81, 0x81ull << 7*8, 0x8, 0x8ull << 7*8, 0x10, 0x10ull << 7*8, 0ull);


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


#endif //CHESSENGINE_BOARD_H

