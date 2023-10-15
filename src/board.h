//
// Created by Robin on 29.06.2022.
//

#include <memory>
#include "chess.h"

#ifndef DORY_BOARD_H
#define DORY_BOARD_H

struct State {
    constexpr State(bool white, bool wcs, bool wcl, bool bcs, bool bcl) :
            whiteToMove{white}, wCastleShort{wcs}, wCastleLong{wcl},
            bCastleShort{bcs}, bCastleLong{bcl} {}

//    constexpr State(const State& other) = default;

    bool whiteToMove;
    bool wCastleShort, wCastleLong;
    bool bCastleShort, bCastleLong;

    unsigned int code() {
        uint8_t state_code = 0;
        if(whiteToMove)   state_code |= 0b10000;
        if(wCastleShort) state_code |= 0b1000;
        if(wCastleLong) state_code |= 0b100;
        if(bCastleShort) state_code |= 0b10;
        if(bCastleLong) state_code |= 0b1;
        return state_code;
    }
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

class Board;

using BoardPtr = std::unique_ptr<Board>;

class Board {
public:
    BB wPawns{0}, bPawns{0}, wKnights{0}, bKnights{0}, wBishops{0}, bBishops{0}, wRooks{0}, bRooks{0}, wQueens{0}, bQueens{0};
    uint8_t wKingSq{0}, bKingSq{0}, enPassantSq{0};

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
    [[nodiscard]] BoardPtr getNextBoard(BB from, BB to) const {
        constexpr bool whiteMoved = state.whiteToMove;
        BB change = from | to;

        // Promotions
        if constexpr (flags == MoveFlag::PromoteQueen) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens | to, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens | to, wKingSq, bKingSq, 0);
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks | to, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks | to, wQueens & ~to, bQueens, wKingSq, bKingSq, 0);
        }
        if constexpr (flags == MoveFlag::PromoteBishop) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops | to, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops | to, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0);
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns & ~from, bPawns, wKnights | to, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns, bPawns & ~from, wKnights & ~to, bKnights | to, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0);
        }

        //Castles
        if constexpr (flags == MoveFlag::ShortCastling) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleShortRookMove<whiteMoved>(), bRooks, wQueens, bQueens, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0);
            return std::make_unique<Board>(wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleShortRookMove<whiteMoved>(), wQueens, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0);
        }
        if constexpr (flags == MoveFlag::LongCastling) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleLongRookMove<whiteMoved>(), bRooks, wQueens, bQueens, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0);
            return std::make_unique<Board>(wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleLongRookMove<whiteMoved>(), wQueens, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0);
        }

        // Silent Moves
        if constexpr (piece == Piece::Pawn) {
            BB epMask = flags == MoveFlag::EnPassantCapture ? ~backward<whiteMoved>(newMask(enPassantSq)) : FULL_BB;
            uint8_t epField = flags == MoveFlag::PawnDoublePush ? singleBitOf(forward<whiteMoved>(from)) : 0;
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns ^ change, bPawns & epMask & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, epField);
            return std::make_unique<Board>(wPawns & epMask & ~to, bPawns ^ change, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, epField);
        }
        if constexpr (piece == Piece::Knight) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns, bPawns & ~to, wKnights ^ change, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns & ~to, bPawns, wKnights & ~to, bKnights ^ change, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0);
        }
        if constexpr (piece == Piece::Bishop) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops ^ change, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops ^ change, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0);
        }
        if constexpr (piece == Piece::Rook) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks ^ change, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks ^ change, wQueens & ~to, bQueens, wKingSq, bKingSq, 0);
        }
        if constexpr (piece == Piece::Queen) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens ^ change, bQueens & ~to, wKingSq, bKingSq, 0);
            return std::make_unique<Board>(wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens ^ change, wKingSq, bKingSq, 0);
        }
        if constexpr (piece == Piece::King) {
            if constexpr (whiteMoved) return std::make_unique<Board>(wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0);
            return std::make_unique<Board>(wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0);
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
std::pair<BoardPtr, State> forkBoard(const Board& board, State state, Move move) {
    unsigned int state_code = state.code();
    switch (state_code) {
        case 0:
            return { board.getNextBoard<toState(0), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(0), flags>() };
        case 1:
            return { board.getNextBoard<toState(1), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(1), flags>() };
        case 2:
            return { board.getNextBoard<toState(2), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(2), flags>() };
        case 3:
            return { board.getNextBoard<toState(3), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(3), flags>() };
        case 4:
            return { board.getNextBoard<toState(4), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(4), flags>() };
        case 5:
            return { board.getNextBoard<toState(5), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(5), flags>() };
        case 6:
            return { board.getNextBoard<toState(6), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(6), flags>() };
        case 7:
            return { board.getNextBoard<toState(7), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(7), flags>() };
        case 8:
            return { board.getNextBoard<toState(8), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(8), flags>() };
        case 9:
            return { board.getNextBoard<toState(9), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(9), flags>() };
        case 10:
            return { board.getNextBoard<toState(10), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(10), flags>() };
        case 11:
            return { board.getNextBoard<toState(11), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(11), flags>() };
        case 12:
            return { board.getNextBoard<toState(12), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(12), flags>() };
        case 13:
            return { board.getNextBoard<toState(13), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(13), flags>() };
        case 14:
            return { board.getNextBoard<toState(14), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(14), flags>() };
        case 15:
            return { board.getNextBoard<toState(15), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(15), flags>() };
        case 16:
            return { board.getNextBoard<toState(16), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(16), flags>() };
        case 17:
            return { board.getNextBoard<toState(17), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(17), flags>() };
        case 18:
            return { board.getNextBoard<toState(18), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(18), flags>() };
        case 19:
            return { board.getNextBoard<toState(19), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(19), flags>() };
        case 20:
            return { board.getNextBoard<toState(20), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(20), flags>() };
        case 21:
            return { board.getNextBoard<toState(21), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(21), flags>() };
        case 22:
            return { board.getNextBoard<toState(22), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(22), flags>() };
        case 23:
            return { board.getNextBoard<toState(23), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(23), flags>() };
        case 24:
            return { board.getNextBoard<toState(24), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(24), flags>() };
        case 25:
            return { board.getNextBoard<toState(25), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(25), flags>() };
        case 26:
            return { board.getNextBoard<toState(26), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(26), flags>() };
        case 27:
            return { board.getNextBoard<toState(27), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(27), flags>() };
        case 28:
            return { board.getNextBoard<toState(28), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(28), flags>() };
        case 29:
            return { board.getNextBoard<toState(29), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(29), flags>() };
        case 30:
            return { board.getNextBoard<toState(30), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(30), flags>() };
        case 31:
            return { board.getNextBoard<toState(31), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(31), flags>() };
        default: throw std::runtime_error("This should never happen");
    }
}

std::pair<BoardPtr, State> forkBoard(const Board& board, State state, Move move) {
    switch (move.piece) {
        case Piece::Pawn:
            switch (move.flags) {
                case MoveFlag::PawnDoublePush:
                    return forkBoard<Piece::Pawn, MoveFlag::PawnDoublePush>(board, state, move);
                case MoveFlag::EnPassantCapture:
                    return forkBoard<Piece::Pawn, MoveFlag::EnPassantCapture>(board, state, move);
                case MoveFlag::PromoteQueen:
                    return forkBoard<Piece::Pawn, MoveFlag::PromoteQueen>(board, state, move);
                case MoveFlag::PromoteRook:
                    return forkBoard<Piece::Pawn, MoveFlag::PromoteRook>(board, state, move);
                case MoveFlag::PromoteBishop:
                    return forkBoard<Piece::Pawn, MoveFlag::PromoteBishop>(board, state, move);
                case MoveFlag::PromoteKnight:
                    return forkBoard<Piece::Pawn, MoveFlag::PromoteKnight>(board, state, move);
            }
            return forkBoard<Piece::Pawn>(board, state, move);
        case Piece::Knight:
            return forkBoard<Piece::Knight>(board, state, move);
        case Piece::Bishop:
            return forkBoard<Piece::Bishop>(board, state, move);
        case Piece::Rook:
            switch (move.flags) {
                case MoveFlag::RemoveShortCastling:
                    return forkBoard<Piece::Rook, MoveFlag::RemoveShortCastling>(board, state, move);
                case MoveFlag::RemoveLongCastling:
                    return forkBoard<Piece::Rook, MoveFlag::RemoveLongCastling>(board, state, move);
            }
            return forkBoard<Piece::Rook>(board, state, move);
        case Piece::Queen:
            return forkBoard<Piece::Queen>(board, state, move);
        case Piece::King:
            switch (move.flags) {
                case MoveFlag::RemoveAllCastling:
                    return forkBoard<Piece::King, MoveFlag::RemoveAllCastling>(board, state, move);
                case MoveFlag::ShortCastling:
                    return forkBoard<Piece::King, MoveFlag::ShortCastling>(board, state, move);
                case MoveFlag::LongCastling:
                    return forkBoard<Piece::King, MoveFlag::LongCastling>(board, state, move);
            }
            return forkBoard<Piece::King>(board, state, move);
    }
    throw std::runtime_error("INVALID PIECE MOVED");
}

#endif //DORY_BOARD_H

