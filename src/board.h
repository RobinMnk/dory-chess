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


constexpr State toState(const uint8_t code) {
    return {(code & 0b10000) != 0, (code & 0b01000) != 0, (code & 0b00100) != 0, (code & 0b00010) != 0, (code & 0b00001) != 0};
}

template<State state>
constexpr uint8_t toCode() {
    uint8_t state_code = 0;
    if(state.whiteToMove)   state_code |= 0b10000;
    if(state.wCastleShort) state_code |= 0b1000;
    if(state.wCastleLong) state_code |= 0b100;
    if(state.bCastleShort) state_code |= 0b10;
    if(state.bCastleLong) state_code |= 0b1;
    return state_code;
}

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
    const BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{0}, bQueens{0};
    const uint8_t wKingSq{0}, bKingSq{0}, enPassantSq{0};

    Board() = default;
    constexpr Board(BB wP, BB bP, BB wN, BB bN, BB wB, BB bB, BB wR, BB bR, BB wQ, BB bQ, uint8_t wK, uint8_t bK, uint8_t ep) :
            wPawns{wP}, bPawns{bP}, wKnights{wN}, bKnights{bN}, wBishops{wB}, bBishops{bB},
            wRooks{wR}, bRooks{bR}, wQueens{wQ}, bQueens{bQ}, wKingSq{wK}, bKingSq{bK}, enPassantSq{ep} {}

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
        if constexpr (whiteToMove) return newMask(wKingSq); else return newMask(bKingSq);
    }

    [[nodiscard]] constexpr BB enPassantBB() const {
        return newMask(enPassantSq);
    }

    [[nodiscard]] constexpr bool hasEnPassant() const {
        return enPassantSq != 0;
    }

    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyPawns() const     { return pawns<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyKnights() const   { return knights<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyBishops() const   { return bishops<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyRooks() const     { return rooks<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyQueens() const    { return queens<!whiteToMove>(); }
    template<bool whiteToMove> [[nodiscard]] constexpr BB enemyKing() const      { return king<!whiteToMove>(); }

    template<bool whiteToMove>
    [[nodiscard]] constexpr int kingSquare() const {
        if constexpr (whiteToMove) return wKingSq;
        else return bKingSq;
    }

    [[nodiscard]] constexpr BB occ() const {
        return wPawns | wKnights | wBishops | wRooks | wQueens | newMask(wKingSq)
                 | bPawns | bKnights | bBishops | bRooks | bQueens | newMask(bKingSq);
    }

    [[nodiscard]] constexpr BB free() const {
        return ~occ();
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr BB allPieces() const {
        if constexpr (whiteToMove) return wPawns | wKnights | wBishops | wRooks | wQueens | newMask(wKingSq);
        else return bPawns | bKnights | bBishops | bRooks | bQueens | newMask(bKingSq);
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
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens | to, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens | to, wKingSq, bKingSq, 0};
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks | to, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks | to, wQueens & ~to, bQueens, wKingSq, bKingSq, 0};
        }
        if constexpr (flags == MoveFlag::PromoteBishop) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops | to, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops | to, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0};
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights | to, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights | to, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0};
        }

        //Castles
        if constexpr (flags == MoveFlag::ShortCastling) {
            if constexpr (whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleShortRookMove<whiteMoved>(), bRooks, wQueens, bQueens, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleShortRookMove<whiteMoved>(), wQueens, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0};
        }
        if constexpr (flags == MoveFlag::LongCastling) {
            if constexpr (whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleLongRookMove<whiteMoved>(), bRooks, wQueens, bQueens, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleLongRookMove<whiteMoved>(), wQueens, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0};
        }

        // Silent Moves
        if constexpr (piece == Piece::Pawn) {
            BB epMask = flags == MoveFlag::EnPassantCapture ? ~backward<whiteMoved>(newMask(enPassantSq)) : FULL_BB;
            uint8_t epField = flags == MoveFlag::PawnDoublePush ? singleBitOf(forward<whiteMoved>(from)) : 0;
            if constexpr (whiteMoved) return {wPawns ^ change, bPawns & epMask & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, epField};
            return {wPawns & epMask & ~to, bPawns ^ change, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, epField};
        }
        if constexpr (piece == Piece::Knight) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights ^ change, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights ^ change, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0};
        }
        if constexpr (piece == Piece::Bishop) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops ^ change, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops ^ change, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0};
        }
        if constexpr (piece == Piece::Rook) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks ^ change, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks ^ change, wQueens & ~to, bQueens, wKingSq, bKingSq, 0};
        }
        if constexpr (piece == Piece::Queen) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens ^ change, bQueens & ~to, wKingSq, bKingSq, 0};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens ^ change, wKingSq, bKingSq, 0};
        }
        if constexpr (piece == Piece::King) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0};
        }
        throw std::exception();
    }
};


constexpr Board STARTBOARD = Board(rank2, rank7, 0x42, 0x42ull << 7*8, 0x24, 0x24ull << 7*8, 0x81, 0x81ull << 7*8, 0x8, 0x8ull << 7*8, 4, 60, 0);


struct ExtendedBoard {
    Board board{};
    uint8_t state_code{};
};

template<State state>
ExtendedBoard getExtendedBoard(const Board& board) {
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
    if constexpr (isWhite) return 0b111ull << STARTBOARD.wKingSq;
    else return 0b111ull << STARTBOARD.bKingSq;
}

template<bool isWhite>
constexpr BB castleLongMask() {
    if constexpr (isWhite) return 0b111ull << (STARTBOARD.wKingSq - 2);
    else return 0b111ull << (STARTBOARD.bKingSq - 2);
}

template<bool isWhite>
constexpr BB castleShortRookMove() {
    if constexpr (isWhite) return 0b101ull << (STARTBOARD.wKingSq + 1);
    else return 0b101ull << (STARTBOARD.bKingSq + 1);
}

template<bool isWhite>
constexpr BB castleLongRookMove() {
    if constexpr (isWhite) return 0b1001ull;
    else return 0b1001ull << (STARTBOARD.bKingSq - 4);
}


template<Piece_t piece, Flag_t flags = MoveFlag::Silent>
std::pair<Board, uint8_t> make_move(const Board& board, uint8_t state_code, Move move) {
    if(state_code == 0) {
        constexpr State state = toState(0);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 1) {
        constexpr State state = toState(1);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 2) {
        constexpr State state = toState(2);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 3) {
        constexpr State state = toState(3);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 4) {
        constexpr State state = toState(4);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 5) {
        constexpr State state = toState(5);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 6) {
        constexpr State state = toState(6);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 7) {
        constexpr State state = toState(7);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 8) {
        constexpr State state = toState(8);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 9) {
        constexpr State state = toState(9);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 10) {
        constexpr State state = toState(10);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 11) {
        constexpr State state = toState(11);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 12) {
        constexpr State state = toState(12);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 13) {
        constexpr State state = toState(13);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 14) {
        constexpr State state = toState(14);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 15) {
        constexpr State state = toState(15);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 16) {
        constexpr State state = toState(16);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 17) {
        constexpr State state = toState(17);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 18) {
        constexpr State state = toState(18);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 19) {
        constexpr State state = toState(19);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 20) {
        constexpr State state = toState(20);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 21) {
        constexpr State state = toState(21);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 22) {
        constexpr State state = toState(22);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 23) {
        constexpr State state = toState(23);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 24) {
        constexpr State state = toState(24);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 25) {
        constexpr State state = toState(25);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 26) {
        constexpr State state = toState(26);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 27) {
        constexpr State state = toState(27);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 28) {
        constexpr State state = toState(28);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 29) {
        constexpr State state = toState(29);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 30) {
        constexpr State state = toState(30);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    if(state_code == 31) {
        constexpr State state = toState(31);
        Board nextBoard = board.getNextBoard<state, piece, flags>(move.from, move.to);
        constexpr State nextState = getNextState<state, flags>();
        return {nextBoard, toCode<nextState>()};
    }
    throw std::runtime_error("INVALID STATE CODE");
}

std::pair<Board, uint8_t> make_move(const Board& board, uint8_t state_code, Move move) {
    switch (move.piece) {
        case Piece::Pawn:
            switch (move.flags) {
                case MoveFlag::PawnDoublePush:
                    return make_move<Piece::Pawn, MoveFlag::PawnDoublePush>(board, state_code, move);
                case MoveFlag::EnPassantCapture:
                    return make_move<Piece::Pawn, MoveFlag::EnPassantCapture>(board, state_code, move);
                case MoveFlag::PromoteQueen:
                    return make_move<Piece::Pawn, MoveFlag::PromoteQueen>(board, state_code, move);
                case MoveFlag::PromoteRook:
                    return make_move<Piece::Pawn, MoveFlag::PromoteRook>(board, state_code, move);
                case MoveFlag::PromoteBishop:
                    return make_move<Piece::Pawn, MoveFlag::PromoteBishop>(board, state_code, move);
                case MoveFlag::PromoteKnight:
                    return make_move<Piece::Pawn, MoveFlag::PromoteKnight>(board, state_code, move);
            }
            return make_move<Piece::Pawn>(board, state_code, move);
        case Piece::Knight:
            return make_move<Piece::Knight>(board, state_code, move);
        case Piece::Bishop:
            return make_move<Piece::Bishop>(board, state_code, move);
        case Piece::Rook:
            switch (move.flags) {
                case MoveFlag::RemoveShortCastling:
                    return make_move<Piece::Rook, MoveFlag::RemoveShortCastling>(board, state_code, move);
                case MoveFlag::RemoveLongCastling:
                    return make_move<Piece::Rook, MoveFlag::RemoveLongCastling>(board, state_code, move);
            }
            return make_move<Piece::Rook>(board, state_code, move);
        case Piece::Queen:
            return make_move<Piece::Queen>(board, state_code, move);
        case Piece::King:
            switch (move.flags) {
                case MoveFlag::RemoveAllCastling:
                    return make_move<Piece::King, MoveFlag::RemoveAllCastling>(board, state_code, move);
                case MoveFlag::ShortCastling:
                    return make_move<Piece::King, MoveFlag::ShortCastling>(board, state_code, move);
                case MoveFlag::LongCastling:
                    return make_move<Piece::King, MoveFlag::LongCastling>(board, state_code, move);
            }
            return make_move<Piece::King>(board, state_code, move);
    }
    throw std::runtime_error("INVALID PIECE MOVED");
}

#endif //DORY_BOARD_H

