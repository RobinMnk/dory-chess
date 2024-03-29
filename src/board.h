//
// Created by Robin on 29.06.2022.
//

#include "chess.h"

#ifndef DORY_BOARD_H
#define DORY_BOARD_H

struct State {
    constexpr State(bool white, bool wcs, bool wcl, bool bcs, bool bcl) :
            whiteToMove{white}, wCastleShort{wcs}, wCastleLong{wcl},
            bCastleShort{bcs}, bCastleLong{bcl} {}

    const bool whiteToMove;
    const bool wCastleShort, wCastleLong;
    const bool bCastleShort, bCastleLong;
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

template<State state>
constexpr bool canCastle() {
    return canCastleShort<state>() || canCastleLong<state>();
}

template<State state, Flag_t flag = MoveFlag::Silent>
constexpr State getNextState() {
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

template<State state>
constexpr uint8_t getStateCode() {
    uint8_t code = 0;
    if constexpr(state.whiteToMove)     code |= 0b10000;
    if constexpr(state.wCastleShort)    code |= 0b1000;
    if constexpr(state.wCastleLong)     code |= 0b100;
    if constexpr(state.bCastleShort)    code |= 0b10;
    if constexpr(state.bCastleLong)     code |= 0b1;
    return code;
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
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyKing() const      { return king<!whiteToMove>(); }

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
        BB change = from | to;

        // Promotions
        if constexpr (flags == MoveFlag::PromoteQueen) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens | to, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens | to, wKing, bKing, 0ull};
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks | to, bRooks & ~to, wQueens, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks | to, wQueens & ~to, bQueens, wKing, bKing, 0ull};
        }
        if constexpr (flags == MoveFlag::PromoteBishop) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops | to, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops | to, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKing, bKing, 0ull};
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights | to, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKing, bKing, 0ull};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights | to, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKing, bKing, 0ull};
        }

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


struct ExtendedBoard {
    Board board{};
    uint8_t state_code{};
};

template<State state>
ExtendedBoard getExtendedBoard(Board& board) {
    return { board, getStateCode<state>() };
}

template<bool isWhite>
static constexpr BB startingKingsideRook() {
    if constexpr (isWhite) return newMask(7);
    else return newMask(63);
}

template<bool isWhite>
static constexpr BB startingQueensideRook() {
    if constexpr (isWhite) return 1ull;
    else return newMask(56);
}

template<bool isWhite>
constexpr BB castleShortMask() {
    if constexpr (isWhite) return 0b111ull << singleBitOf(STARTBOARD.wKing);
    else return 0b111ull << singleBitOf(STARTBOARD.bKing);
}

template<bool isWhite>
constexpr BB castleLongMask() {
    if constexpr (isWhite) return 0b111ull << (singleBitOf(STARTBOARD.wKing) - 2);
    else return 0b111ull << (singleBitOf(STARTBOARD.bKing) - 2);
}

template<bool isWhite>
constexpr BB castleShortRookMove() {
    if constexpr (isWhite) return 0b101ull << (singleBitOf(STARTBOARD.wKing) + 1);
    else return 0b101ull << (singleBitOf(STARTBOARD.bKing) + 1);
}

template<bool isWhite>
constexpr BB castleLongRookMove() {
    if constexpr (isWhite) return 0b1001ull;
    else return 0b1001ull << (singleBitOf(STARTBOARD.bKing) - 4);
}


#endif //DORY_BOARD_H

