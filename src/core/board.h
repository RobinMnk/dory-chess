//
// Created by Robin on 29.06.2022.
//

#include "chess.h"

#ifndef DORY_BOARD_H
#define DORY_BOARD_H

//struct State {
//    bool whiteToMove{};
//    bool wCastleShort{}, wCastleLong{};
//    bool bCastleShort{}, bCastleLong{};
//
//    constexpr State(bool white, bool wcs, bool wcl, bool bcs, bool bcl) :
//            whiteToMove{white}, wCastleShort{wcs}, wCastleLong{wcl},
//            bCastleShort{bcs}, bCastleLong{bcl} {}
//
//    [[nodiscard]] unsigned int code() const {
//        uint8_t state_code = 0;
//        if(whiteToMove)   state_code |= 0b10000;
//        if(wCastleShort) state_code |= 0b1000;
//        if(wCastleLong) state_code |= 0b100;
//        if(bCastleShort) state_code |= 0b10;
//        if(bCastleLong) state_code |= 0b1;
//        return state_code;
//    }
//
//    void update(Flag_t flag) {
//        whiteToMove = !whiteToMove;
//
//        if (flag == MoveFlag::RemoveShortCastling) {
//            if (whiteToMove) { wCastleShort = false; }
//            else { bCastleShort = false; }
//        }
//        else if (flag == MoveFlag::RemoveLongCastling) {
//            if (whiteToMove) { wCastleLong = false; }
//            else { bCastleLong = false; }
//        }
//        else if (flag == MoveFlag::RemoveAllCastling || flag == MoveFlag::ShortCastling || flag == MoveFlag::LongCastling) {
//            if (whiteToMove) { wCastleShort = wCastleLong = false; }
//            else { bCastleShort = bCastleLong = false; }
//        }
//    }
//};
//constexpr State STARTSTATE = State(true, true, true, true, true);


//constexpr State toState(const uint8_t code) {
//    return {(code & 0b10000) != 0, (code & 0b01000) != 0, (code & 0b00100) != 0, (code & 0b00010) != 0, (code & 0b00001) != 0};
//}
//
//template<State state>
//constexpr uint8_t toCode() {
//    uint8_t state_code = 0;
//    if(state.whiteToMove)   state_code |= 0b10000;
//    if(state.wCastleShort) state_code |= 0b1000;
//    if(state.wCastleLong) state_code |= 0b100;
//    if(state.bCastleShort) state_code |= 0b10;
//    if(state.bCastleLong) state_code |= 0b1;
//    return state_code;
//}

/// Castling Masks

const uint8_t wCastleShortMask = 0b1000;
const uint8_t wCastleLongMask = 0b10;
const uint8_t wCastleMask = wCastleLongMask | wCastleShortMask;
const uint8_t bCastleShortMask = 0b100;
const uint8_t bCastleLongMask = 0b1;
const uint8_t bCastleMask = bCastleLongMask | bCastleShortMask;

//template<bool whiteToMove>
//constexpr bool canCastleShort(uint8_t state_code) {
//}
//
//template<bool whiteToMove>
//constexpr bool canCastleLong(uint8_t state_code) {
//}

//template<State state, Flag_t flag = MoveFlag::Silent>
//constexpr State getNextState() {
//    if constexpr (flag == MoveFlag::RemoveShortCastling) {
//        if constexpr (state.whiteToMove) return {false, false, state.wCastleLong, state.bCastleShort, state.bCastleLong};
//        else return {true, state.wCastleShort, state.wCastleLong, false, state.bCastleLong};
//    }
//
//    if constexpr (flag == MoveFlag::RemoveLongCastling) {
//        if constexpr (state.whiteToMove) return {false, state.wCastleShort, false, state.bCastleShort, state.bCastleLong};
//        else return {true, state.wCastleShort, state.wCastleLong, state.bCastleShort, false};
//    }
//
//    if constexpr (flag == MoveFlag::RemoveAllCastling || flag == MoveFlag::ShortCastling || flag == MoveFlag::LongCastling) {
//        if constexpr (state.whiteToMove) return {false, false, false, state.bCastleShort, state.bCastleLong};
//        else return {true, state.wCastleShort, state.wCastleLong, false, false};
//    }
//
//    return {!state.whiteToMove, state.wCastleShort, state.wCastleLong, state.bCastleShort, state.bCastleLong};
//}

//template<State state>
//constexpr uint8_t getStateCode() {
//    uint8_t code = 0;
//    if constexpr(state.whiteToMove)     code |= 0b10000;
//    if constexpr(state.wCastleShort)    code |= 0b1000;
//    if constexpr(state.wCastleLong)     code |= 0b100;
//    if constexpr(state.bCastleShort)    code |= 0b10;
//    if constexpr(state.bCastleLong)     code |= 0b1;
//    return code;
//}


template<bool isWhite>
constexpr BB castleShortRookMove();

template<bool isWhite>
constexpr BB castleLongRookMove();

class Board {
public:
    BB wPawns{}, bPawns{}, wKnights{}, bKnights{}, wBishops{}, bBishops{}, wRooks{}, bRooks{}, wQueens{}, bQueens{};
    uint8_t wKingSq{}, bKingSq{}, enPassantSq{}, castling{}; // Optimization potential: merge (castling and ep) and king squares into same byte

    Board() = default;
    constexpr Board(BB wP, BB bP, BB wN, BB bN, BB wB, BB bB, BB wR, BB bR, BB wQ, BB bQ, uint8_t wK, uint8_t bK, uint8_t ep, uint8_t cs) :
            wPawns{wP}, bPawns{bP}, wKnights{wN}, bKnights{bN}, wBishops{wB}, bBishops{bB},
            wRooks{wR}, bRooks{bR}, wQueens{wQ}, bQueens{bQ}, wKingSq{wK}, bKingSq{bK}, enPassantSq{ep}, castling{cs} {}

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

    [[nodiscard]] constexpr uint8_t castlingRights() const { return castling; }

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

    template<Piece_t piece, bool whiteToMove>
    [[nodiscard]] BB getPieceBB() const {
        if constexpr (piece == Piece::Pawn)
            return pawns<whiteToMove>();
        else if constexpr (piece == Piece::Knight)
            return knights<whiteToMove>();
        else if constexpr (piece == Piece::Bishop)
            return bishops<whiteToMove>();
        else if constexpr (piece == Piece::Rook)
            return rooks<whiteToMove>();
        else if constexpr (piece == Piece::Queen)
            return queens<whiteToMove>();
        else if constexpr (piece == Piece::King)
            return king<whiteToMove>();
        return 0;
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

    template<bool whiteToMove>
    bool isCapture(Move& move) const {
        return move.to() & enemyPieces<whiteToMove>();
    }

    bool isCapture(bool whiteToMove, Move& move) const {
        return whiteToMove ? isCapture<true>(move) : isCapture<false>(move);
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr bool canCastleShort() const {
        if constexpr (whiteToMove) return castling & wCastleShortMask;
        else return castling & bCastleShortMask;
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr bool canCastleLong() const {
        if constexpr (whiteToMove) return castling & wCastleLongMask;
        else return castling & bCastleLongMask;
    }

    template<bool whiteToMove>
    [[nodiscard]] constexpr bool canCastle() const {
        if constexpr (whiteToMove) return castling & wCastleMask;
        else return castling & bCastleMask;
    }


    template<bool whiteMoved, Piece_t piece, Flag_t flags=MoveFlag::Silent>
    [[nodiscard]] constexpr Board fork(BB from, BB to) const {
        BB change = from | to;
        uint8_t cs = castling;

        if constexpr (flags == MoveFlag::RemoveShortCastling) {
            if constexpr (whiteMoved) { cs &= ~wCastleShortMask; }
            else { cs &= ~bCastleShortMask; }
        }
        else if constexpr (flags == MoveFlag::RemoveLongCastling) {
            if constexpr (whiteMoved) { cs &= ~wCastleLongMask; }
            else { cs &= ~bCastleLongMask; }
        }
        else if constexpr (flags == MoveFlag::RemoveAllCastling || flags == MoveFlag::ShortCastling || flags == MoveFlag::LongCastling) {
            if constexpr (whiteMoved) { cs &= ~wCastleMask; }
            else { cs &= ~bCastleMask; }
        }

        // Promotions
        if constexpr (flags == MoveFlag::PromoteQueen) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens | to, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens | to, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks | to, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks | to, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (flags == MoveFlag::PromoteBishop) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops | to, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops | to, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            if constexpr (whiteMoved) return {wPawns & ~from, bPawns, wKnights | to, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights | to, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }

        // Castles
        if constexpr (flags == MoveFlag::ShortCastling) {
            if constexpr (whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleShortRookMove<whiteMoved>(), bRooks, wQueens, bQueens, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0, cs};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleShortRookMove<whiteMoved>(), wQueens, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0, cs};
        }
        if constexpr (flags == MoveFlag::LongCastling) {
            if constexpr (whiteMoved) return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks ^ castleLongRookMove<whiteMoved>(), bRooks, wQueens, bQueens, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0, cs};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks, bRooks ^ castleLongRookMove<whiteMoved>(), wQueens, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0, cs};
        }

        // Silent Moves
        if constexpr (piece == Piece::Pawn) {
            BB epMask = flags == MoveFlag::EnPassantCapture ? ~backward<whiteMoved>(newMask(enPassantSq)) : FULL_BB;
            uint8_t epField = flags == MoveFlag::PawnDoublePush ? singleBitOf(forward<whiteMoved>(from)) : 0;
            if constexpr (whiteMoved) return {wPawns ^ change, bPawns & epMask & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, epField, cs};
            return {wPawns & epMask & ~to, bPawns ^ change, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, epField, cs};
        }
        if constexpr (piece == Piece::Knight) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights ^ change, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights ^ change, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == Piece::Bishop) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops ^ change, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops ^ change, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == Piece::Rook) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks ^ change, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks ^ change, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == Piece::Queen) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens ^ change, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens ^ change, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == Piece::King) {
            if constexpr (whiteMoved) return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks, bRooks & ~to, wQueens, bQueens & ~to, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0, cs};
        }
        throw std::exception();
    }

    template<bool whiteMoved, Piece_t piece, Flag_t flags>
    void makeMove(BB from, BB to) {
        BB change = from | to;

        if constexpr (flags == MoveFlag::RemoveShortCastling) {
            if constexpr (whiteMoved) { castling &= ~wCastleShortMask; }
            else { castling &= ~bCastleShortMask; }
        }
        else if constexpr (flags == MoveFlag::RemoveLongCastling) {
            if constexpr (whiteMoved) { castling &= ~wCastleLongMask; }
            else { castling &= ~bCastleLongMask; }
        }
        else if constexpr (flags == MoveFlag::RemoveAllCastling || flags == MoveFlag::ShortCastling || flags == MoveFlag::LongCastling) {
            if constexpr (whiteMoved) { castling &= ~wCastleMask; }
            else { castling &= ~bCastleMask; }
        }

        // Promotions
        if constexpr (flags == MoveFlag::PromoteQueen) {
            if constexpr (whiteMoved) { wPawns &= ~from; bKnights &= ~to; bBishops &= ~to; bRooks &= ~to; wQueens |= to; bQueens &= ~to; }
            else { bPawns &= ~from; wKnights &= ~to; wBishops &= ~to; wRooks &= ~to; wQueens &= ~to; bQueens |= to; }
            return;
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            if constexpr (whiteMoved) { wPawns &= ~from; bKnights &= ~to; bBishops &= ~to; wRooks |= to; bRooks &= ~to; bQueens &= ~to; }
            else { bPawns &= ~from; wKnights &= ~to; wBishops &= ~to; wRooks &= ~to; bRooks |= to; wQueens &= ~to; }
            return;
        }
        if constexpr (flags == MoveFlag::PromoteBishop) {
            if constexpr (whiteMoved) { wPawns &= ~from; bKnights &= ~to; wBishops |= to; bBishops &= ~to; bRooks &= ~to; bQueens &= ~to; }
            else { bPawns &= ~from; wKnights &= ~to; wBishops &= ~to; bBishops |= to; wRooks &= ~to; wQueens &= ~to; }
            return;
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            if constexpr (whiteMoved) { wPawns &= ~from; wKnights |= to; bKnights &= ~to; bBishops &= ~to; bRooks &= ~to; bQueens &= ~to; }
            else { bPawns &= ~from; wKnights &= ~to; bKnights |= to, wBishops &= ~to; wRooks &= ~to; wQueens &= ~to; }
            return;
        }

        //Castles
        if constexpr (flags == MoveFlag::ShortCastling) {
            if constexpr (whiteMoved) { wRooks ^= castleShortRookMove<whiteMoved>(); wKingSq = static_cast<uint8_t>(singleBitOf(to)); }
            else { bRooks ^= castleShortRookMove<whiteMoved>(); bKingSq = static_cast<uint8_t>(singleBitOf(to)); }
            return;
        }
        if constexpr (flags == MoveFlag::LongCastling) {
            if constexpr (whiteMoved) { wRooks ^= castleLongRookMove<whiteMoved>(), wKingSq = static_cast<uint8_t>(singleBitOf(to)); }
            else { bRooks ^= castleLongRookMove<whiteMoved>(), bKingSq = static_cast<uint8_t>(singleBitOf(to)); }
            return;
        }

        // Silent Moves
        if constexpr (piece == Piece::Pawn) {
            BB epMask = flags == MoveFlag::EnPassantCapture ? ~backward<whiteMoved>(newMask(enPassantSq)) : FULL_BB;
            enPassantSq = flags == MoveFlag::PawnDoublePush ? singleBitOf(forward<whiteMoved>(from)) : 0;
            if constexpr (whiteMoved) { wPawns ^= change; bPawns &= epMask & ~to; bKnights &= ~to; bBishops &= ~to; bRooks &= ~to; bQueens &= ~to; }
            else { wPawns &= epMask & ~to; bPawns ^= change; wKnights &= ~to; wBishops &= ~to; wRooks &= ~to; wQueens &= ~to; }
            return;
        }
        if constexpr (piece == Piece::Knight) {
            if constexpr (whiteMoved) { bPawns &= ~to; wKnights ^= change; bKnights &= ~to; bBishops &= ~to; bRooks &= ~to; bQueens &= ~to; }
            else { wPawns &= ~to; wKnights &= ~to; bKnights ^= change; wBishops &= ~to; wRooks &= ~to; wQueens &= ~to; }
            return;
        }
        if constexpr (piece == Piece::Bishop) {
            if constexpr (whiteMoved) { bPawns &= ~to; bKnights &= ~to; wBishops ^= change; bBishops &= ~to; bRooks &= ~to; bQueens &= ~to; }
            else { wPawns &= ~to; wKnights &= ~to; wBishops &= ~to; bBishops ^= change; wRooks &= ~to; wQueens &= ~to; }
            return;
        }
        if constexpr (piece == Piece::Rook) {
            if constexpr (whiteMoved)  { bPawns &= ~to; bKnights &= ~to; bBishops &= ~to; wRooks ^= change; bRooks &= ~to; bQueens &= ~to; }
            else { wPawns &= ~to; wKnights &= ~to; wBishops &= ~to; wRooks &= ~to; bRooks ^= change; wQueens &= ~to; }
            return;
        }
        if constexpr (piece == Piece::Queen) {
            if constexpr (whiteMoved) { bPawns &= ~to; bKnights &= ~to; bBishops &= ~to; bRooks &= ~to; wQueens ^= change; bQueens &= ~to; }
            else { wPawns &= ~to; wKnights &= ~to; wBishops &= ~to; wRooks &= ~to; wQueens &= ~to; bQueens ^= change; }
            return;
        }
        if constexpr (piece == Piece::King) {
            if constexpr (whiteMoved) { bPawns &= ~to; bKnights &= ~to; bBishops &= ~to; bRooks &= ~to; bQueens &= ~to, wKingSq = static_cast<uint8_t>(singleBitOf(to)); }
            else { wPawns &= ~to; wKnights &= ~to; wBishops &= ~to; wRooks &= ~to; wQueens &= ~to; bKingSq = static_cast<uint8_t>(singleBitOf(to)); }
            return;
        }
        throw std::exception();
    }

    template<bool whiteToMove>
    void makeMove(Move move) {
        switch (move.piece) {
            case Piece::Pawn:
                switch (move.flags) {
                    case MoveFlag::PawnDoublePush:
                        makeMove<whiteToMove, Piece::Pawn, MoveFlag::PawnDoublePush>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::EnPassantCapture:
                        makeMove<whiteToMove, Piece::Pawn, MoveFlag::EnPassantCapture>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::PromoteQueen:
                        makeMove<whiteToMove, Piece::Pawn, MoveFlag::PromoteQueen>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::PromoteRook:
                        makeMove<whiteToMove, Piece::Pawn, MoveFlag::PromoteRook>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::PromoteBishop:
                        makeMove<whiteToMove, Piece::Pawn, MoveFlag::PromoteBishop>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::PromoteKnight:
                        makeMove<whiteToMove, Piece::Pawn, MoveFlag::PromoteKnight>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                }
                makeMove<whiteToMove, Piece::Pawn>(newMask(move.fromIndex), newMask(move.toIndex)); return;
            case Piece::Knight:
                makeMove<whiteToMove, Piece::Knight>(newMask(move.fromIndex), newMask(move.toIndex)); return;
            case Piece::Bishop:
                makeMove<whiteToMove, Piece::Bishop>(newMask(move.fromIndex), newMask(move.toIndex)); return;
            case Piece::Rook:
                switch (move.flags) {
                    case MoveFlag::RemoveShortCastling:
                        makeMove<whiteToMove, Piece::Rook, MoveFlag::RemoveShortCastling>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::RemoveLongCastling:
                        makeMove<whiteToMove, Piece::Rook, MoveFlag::RemoveLongCastling>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                }
                makeMove<whiteToMove, Piece::Rook>(newMask(move.fromIndex), newMask(move.toIndex)); return;
            case Piece::Queen:
                makeMove<whiteToMove, Piece::Queen>(newMask(move.fromIndex), newMask(move.toIndex)); return;
            case Piece::King:
                switch (move.flags) {
                    case MoveFlag::RemoveAllCastling:
                        makeMove<whiteToMove, Piece::King, MoveFlag::RemoveAllCastling>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::ShortCastling:
                        makeMove<whiteToMove, Piece::King, MoveFlag::ShortCastling>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                    case MoveFlag::LongCastling:
                        makeMove<whiteToMove, Piece::King, MoveFlag::LongCastling>(newMask(move.fromIndex), newMask(move.toIndex)); return;
                }
                makeMove<whiteToMove, Piece::King>(newMask(move.fromIndex), newMask(move.toIndex)); return;
        }
        throw std::runtime_error("INVALID PIECE MOVED");
    }

    template<bool whiteToMove>
    [[nodiscard]] Board fork(const Move& move) const {
        switch (move.piece) {
            case Piece::Pawn:
                switch (move.flags) {
                    case MoveFlag::PawnDoublePush:
                        return fork<whiteToMove, Piece::Pawn, MoveFlag::PawnDoublePush>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::EnPassantCapture:
                        return fork<whiteToMove, Piece::Pawn, MoveFlag::EnPassantCapture>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::PromoteQueen:
                        return fork<whiteToMove, Piece::Pawn, MoveFlag::PromoteQueen>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::PromoteRook:
                        return fork<whiteToMove, Piece::Pawn, MoveFlag::PromoteRook>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::PromoteBishop:
                        return fork<whiteToMove, Piece::Pawn, MoveFlag::PromoteBishop>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::PromoteKnight:
                        return fork<whiteToMove, Piece::Pawn, MoveFlag::PromoteKnight>(newMask(move.fromIndex), newMask(move.toIndex));
                }
                return fork<whiteToMove, Piece::Pawn>(newMask(move.fromIndex), newMask(move.toIndex));
            case Piece::Knight:
                return fork<whiteToMove, Piece::Knight>(newMask(move.fromIndex), newMask(move.toIndex));
            case Piece::Bishop:
                return fork<whiteToMove, Piece::Bishop>(newMask(move.fromIndex), newMask(move.toIndex));
            case Piece::Rook:
                switch (move.flags) {
                    case MoveFlag::RemoveShortCastling:
                        return fork<whiteToMove, Piece::Rook, MoveFlag::RemoveShortCastling>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::RemoveLongCastling:
                        return fork<whiteToMove, Piece::Rook, MoveFlag::RemoveLongCastling>(newMask(move.fromIndex), newMask(move.toIndex));
                }
                return fork<whiteToMove, Piece::Rook>(newMask(move.fromIndex), newMask(move.toIndex));
            case Piece::Queen:
                return fork<whiteToMove, Piece::Queen>(newMask(move.fromIndex), newMask(move.toIndex));
            case Piece::King:
                switch (move.flags) {
                    case MoveFlag::RemoveAllCastling:
                        return fork<whiteToMove, Piece::King, MoveFlag::RemoveAllCastling>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::ShortCastling:
                        return fork<whiteToMove, Piece::King, MoveFlag::ShortCastling>(newMask(move.fromIndex), newMask(move.toIndex));
                    case MoveFlag::LongCastling:
                        return fork<whiteToMove, Piece::King, MoveFlag::LongCastling>(newMask(move.fromIndex), newMask(move.toIndex));
                }
                return fork<whiteToMove, Piece::King>(newMask(move.fromIndex), newMask(move.toIndex));
        }
        throw std::runtime_error("INVALID PIECE MOVED");
    }
};


constexpr Board STARTBOARD = Board(rank2, rank7, 0x42, 0x42ull << 7*8, 0x24, 0x24ull << 7*8, 0x81, 0x81ull << 7*8, 0x8, 0x8ull << 7*8, 4, 60, 0, 0b11111);


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


//template<Piece_t piece, Flag_t flags = MoveFlag::Silent>
//std::pair<Board, State> fork(const Board& board, State state, Move move) {
//    unsigned int state_code = state.code();
//    switch (state_code) {
//        case 0:
//            return { board.fork<toState(0), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(0), flags>() };
//        case 1:
//            return { board.fork<toState(1), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(1), flags>() };
//        case 2:
//            return { board.fork<toState(2), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(2), flags>() };
//        case 3:
//            return { board.fork<toState(3), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(3), flags>() };
//        case 4:
//            return { board.fork<toState(4), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(4), flags>() };
//        case 5:
//            return { board.fork<toState(5), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(5), flags>() };
//        case 6:
//            return { board.fork<toState(6), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(6), flags>() };
//        case 7:
//            return { board.fork<toState(7), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(7), flags>() };
//        case 8:
//            return { board.fork<toState(8), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(8), flags>() };
//        case 9:
//            return { board.fork<toState(9), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(9), flags>() };
//        case 10:
//            return { board.fork<toState(10), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(10), flags>() };
//        case 11:
//            return { board.fork<toState(11), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(11), flags>() };
//        case 12:
//            return { board.fork<toState(12), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(12), flags>() };
//        case 13:
//            return { board.fork<toState(13), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(13), flags>() };
//        case 14:
//            return { board.fork<toState(14), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(14), flags>() };
//        case 15:
//            return { board.fork<toState(15), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(15), flags>() };
//        case 16:
//            return { board.fork<toState(16), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(16), flags>() };
//        case 17:
//            return { board.fork<toState(17), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(17), flags>() };
//        case 18:
//            return { board.fork<toState(18), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(18), flags>() };
//        case 19:
//            return { board.fork<toState(19), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(19), flags>() };
//        case 20:
//            return { board.fork<toState(20), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(20), flags>() };
//        case 21:
//            return { board.fork<toState(21), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(21), flags>() };
//        case 22:
//            return { board.fork<toState(22), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(22), flags>() };
//        case 23:
//            return { board.fork<toState(23), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(23), flags>() };
//        case 24:
//            return { board.fork<toState(24), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(24), flags>() };
//        case 25:
//            return { board.fork<toState(25), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(25), flags>() };
//        case 26:
//            return { board.fork<toState(26), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(26), flags>() };
//        case 27:
//            return { board.fork<toState(27), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(27), flags>() };
//        case 28:
//            return { board.fork<toState(28), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(28), flags>() };
//        case 29:
//            return { board.fork<toState(29), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(29), flags>() };
//        case 30:
//            return { board.fork<toState(30), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(30), flags>() };
//        case 31:
//            return { board.fork<toState(31), piece, flags>(newMask(move.fromIndex), newMask(move.toIndex)), getNextState<toState(31), flags>() };
//        default: throw std::runtime_error("This should never happen");
//    }
//}

//std::pair<Board, State> fork(const Board& board, State state, Move move) {
//    switch (move.piece) {
//        case Piece::Pawn:
//            switch (move.flags) {
//                case MoveFlag::PawnDoublePush:
//                    return fork<Piece::Pawn, MoveFlag::PawnDoublePush>(board, state, move);
//                case MoveFlag::EnPassantCapture:
//                    return fork<Piece::Pawn, MoveFlag::EnPassantCapture>(board, state, move);
//                case MoveFlag::PromoteQueen:
//                    return fork<Piece::Pawn, MoveFlag::PromoteQueen>(board, state, move);
//                case MoveFlag::PromoteRook:
//                    return fork<Piece::Pawn, MoveFlag::PromoteRook>(board, state, move);
//                case MoveFlag::PromoteBishop:
//                    return fork<Piece::Pawn, MoveFlag::PromoteBishop>(board, state, move);
//                case MoveFlag::PromoteKnight:
//                    return fork<Piece::Pawn, MoveFlag::PromoteKnight>(board, state, move);
//            }
//            return fork<Piece::Pawn>(board, state, move);
//        case Piece::Knight:
//            return fork<Piece::Knight>(board, state, move);
//        case Piece::Bishop:
//            return fork<Piece::Bishop>(board, state, move);
//        case Piece::Rook:
//            switch (move.flags) {
//                case MoveFlag::RemoveShortCastling:
//                    return fork<Piece::Rook, MoveFlag::RemoveShortCastling>(board, state, move);
//                case MoveFlag::RemoveLongCastling:
//                    return fork<Piece::Rook, MoveFlag::RemoveLongCastling>(board, state, move);
//            }
//            return fork<Piece::Rook>(board, state, move);
//        case Piece::Queen:
//            return fork<Piece::Queen>(board, state, move);
//        case Piece::King:
//            switch (move.flags) {
//                case MoveFlag::RemoveAllCastling:
//                    return fork<Piece::King, MoveFlag::RemoveAllCastling>(board, state, move);
//                case MoveFlag::ShortCastling:
//                    return fork<Piece::King, MoveFlag::ShortCastling>(board, state, move);
//                case MoveFlag::LongCastling:
//                    return fork<Piece::King, MoveFlag::LongCastling>(board, state, move);
//            }
//            return fork<Piece::King>(board, state, move);
//    }
//    throw std::runtime_error("INVALID PIECE MOVED");
//}

#endif //DORY_BOARD_H

