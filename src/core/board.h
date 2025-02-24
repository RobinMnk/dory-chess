//
// Created by Robin on 29.06.2022.
//

#ifndef DORY_BOARD_H
#define DORY_BOARD_H

#include "chess.h"

namespace Dory {

    // Castling Masks
    const uint8_t wCastleShortMask = 0b1000;
    const uint8_t wCastleLongMask = 0b10;
    const uint8_t wCastleMask = wCastleLongMask | wCastleShortMask;
    const uint8_t bCastleShortMask = 0b100;
    const uint8_t bCastleLongMask = 0b1;
    const uint8_t bCastleMask = bCastleLongMask | bCastleShortMask;

    // Forward declarations
    template<bool isWhite>
    constexpr BB castleShortRookMove();

    template<bool isWhite>
    constexpr BB castleLongRookMove();

    struct RestoreInfo {
        uint8_t epSquare;
        uint8_t castling;
        Piece_t captured;
        // Add halfmove counter etc
    };

    struct Board {
        BB wPawns{}, bPawns{}, wKnights{}, bKnights{}, wBishops{}, bBishops{}, wRooks{}, bRooks{}, wQueens{}, bQueens{};
        uint8_t wKingSq{}, bKingSq{}, enPassantSq{}, castling{}; // Optimization potential: merge (castling and ep) and king squares into same byte

        Board() = default;

        constexpr Board(BB wP, BB bP, BB wN, BB bN, BB wB, BB bB, BB wR, BB bR, BB wQ, BB bQ, uint8_t wK, uint8_t bK,
                        uint8_t ep, uint8_t cs) :
                wPawns{wP}, bPawns{bP}, wKnights{wN}, bKnights{bN}, wBishops{wB}, bBishops{bB},
                wRooks{wR}, bRooks{bR}, wQueens{wQ}, bQueens{bQ}, wKingSq{wK}, bKingSq{bK}, enPassantSq{ep},
                castling{cs} {}

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB pawns() const {
            if constexpr (whiteToMove) return wPawns; else return bPawns;
        }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB knights() const {
            if constexpr (whiteToMove) return wKnights; else return bKnights;
        }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB bishops() const {
            if constexpr (whiteToMove) return wBishops; else return bBishops;
        }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB rooks() const {
            if constexpr (whiteToMove) return wRooks; else return bRooks;
        }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB queens() const {
            if constexpr (whiteToMove) return wQueens; else return bQueens;
        }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB king() const {
            if constexpr (whiteToMove) return newMask(wKingSq); else return newMask(bKingSq);
        }

        [[nodiscard]] constexpr BB enPassantBB() const {
            return newMask(enPassantSq);
        }

        [[nodiscard]] constexpr bool hasEnPassant() const {
            return enPassantSq != 0;
        }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB enemyPawns() const { return pawns<!whiteToMove>(); }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB enemyKnights() const { return knights<!whiteToMove>(); }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB enemyBishops() const { return bishops<!whiteToMove>(); }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB enemyRooks() const { return rooks<!whiteToMove>(); }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB enemyQueens() const { return queens<!whiteToMove>(); }

        template<bool whiteToMove>
        [[nodiscard]] constexpr BB enemyKing() const { return king<!whiteToMove>(); }

        [[nodiscard]] constexpr uint8_t castlingRights() const { return castling; }

        template<bool whiteToMove>
        [[nodiscard]] constexpr uint8_t kingSquare() const {
            if constexpr (whiteToMove) return wKingSq;
            else return bKingSq;
        }

        template<bool whiteToMove>
        [[nodiscard]] constexpr int enemyKingSquare() const {
            if constexpr (whiteToMove) return bKingSq;
            else return wKingSq;
        }

        [[nodiscard]] constexpr BB occ() const {
            return wPawns | wKnights | wBishops | wRooks | wQueens | newMask(wKingSq)
                   | bPawns | bKnights | bBishops | bRooks | bQueens | newMask(bKingSq);
        }

        [[nodiscard]] constexpr BB free() const {
            return ~occ();
        }

        template<Piece_t piece, bool whiteToMove>
        [[nodiscard]] constexpr BB getPieceBB() const {
            if constexpr (piece == PIECE_Pawn)
                return pawns<whiteToMove>();
            else if constexpr (piece == PIECE_Knight)
                return knights<whiteToMove>();
            else if constexpr (piece == PIECE_Bishop)
                return bishops<whiteToMove>();
            else if constexpr (piece == PIECE_Rook)
                return rooks<whiteToMove>();
            else if constexpr (piece == PIECE_Queen)
                return queens<whiteToMove>();
            else if constexpr (piece == PIECE_King)
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
        [[nodiscard]] constexpr bool isCapture(Move move) const {
            return hasBitAt(enemyPieces<whiteToMove>(), move.toIndex);
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

        template<bool whiteToMove>
        [[nodiscard]] constexpr Piece_t getPieceAt(BB sq) const {
            if (sq & pawns<whiteToMove>()) return PIECE_Pawn;
            else if (sq & knights<whiteToMove>()) return PIECE_Knight;
            else if (sq & bishops<whiteToMove>()) return PIECE_Bishop;
            else if (sq & rooks<whiteToMove>()) return PIECE_Rook;
            else if (sq & queens<whiteToMove>()) return PIECE_Queen;
            return PIECE_None;
        }

        template<bool whiteMoved, Piece_t piece, Flag_t flags = MOVEFLAG_Silent>
        [[nodiscard]] constexpr Board fork(BB from, BB to) const;

        template<bool whiteToMove>
        [[nodiscard]] constexpr Board fork(const Move &move) const;

        template<bool whiteMoved, Piece_t piece, Flag_t flags=MOVEFLAG_Silent>
        RestoreInfo makeMove(BB from, BB to);

        template<bool white, Piece_t captured>
        inline void restorePiece(BB sq) {
            if constexpr (white) {
                if constexpr (captured == PIECE_Pawn) {
                    wPawns |= sq;
                } else if constexpr (captured == PIECE_Knight) {
                    wKnights |= sq;
                } else if constexpr (captured == PIECE_Bishop) {
                    wBishops |= sq;
                } else if constexpr (captured == PIECE_Rook) {
                    wRooks |= sq;
                } else if constexpr (captured == PIECE_Queen) {
                    wQueens |= sq;
                }
            } else {
                if constexpr (captured == PIECE_Pawn) {
                    bPawns |= sq;
                } else if constexpr (captured == PIECE_Knight) {
                    bKnights |= sq;
                } else if constexpr (captured == PIECE_Bishop) {
                    bBishops |= sq;
                } else if constexpr (captured == PIECE_Rook) {
                    bRooks |= sq;
                } else if constexpr (captured == PIECE_Queen) {
                    bQueens |= sq;
                }
            }
        }

        template<bool whiteMoved, Piece_t piece, Flag_t flags=MOVEFLAG_Silent, Piece_t captured>
        void unmakeMove(BB from, BB to, RestoreInfo ri) ;

        template<bool whiteMoved, Piece_t piece, Flag_t flags>
        void unmakeMove(BB from, BB to, RestoreInfo ri);

        template<bool whiteToMove>
        void makeMove(Move move);

        void makeMove(Move move, bool whiteToMove) {
            if(whiteToMove) makeMove<true>(move);
            else makeMove<false>(move);
        }

        bool operator==(const Board& other) const = default;

        bool operator!=(const Board& other) const = default;
    }; // struct Board


    // - - - - - - - - - Some Board and Move constants - - - - - - - - -

    constexpr Board STARTBOARD = Board(rank2, rank7, 0x42, 0x42ull << 7 * 8, 0x24, 0x24ull << 7 * 8, 0x81,
                                       0x81ull << 7 * 8, 0x8, 0x8ull << 7 * 8, 4, 60, 0, 0b11111);


    template<bool isWhite>
    constexpr BB startingKingsideRook() {
        if constexpr (isWhite) return newMask(7);
        else return newMask(63);
    }

    template<bool isWhite>
    constexpr BB startingQueensideRook() {
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

    // - - - - - - - - - Out-of-line definitions for Board - - - - - - - - -

    template<bool whiteMoved, Piece_t piece, Flag_t flags>
    constexpr Board Board::fork(BB from, BB to) const {
        BB change = from | to;
        uint8_t cs = castling;

        if constexpr (flags == MOVEFLAG_RemoveShortCastling) {
            if constexpr (whiteMoved) { cs &= ~wCastleShortMask; }
            else { cs &= ~bCastleShortMask; }
        } else if constexpr (flags == MOVEFLAG_RemoveLongCastling) {
            if constexpr (whiteMoved) { cs &= ~wCastleLongMask; }
            else { cs &= ~bCastleLongMask; }
        } else if constexpr (flags == MOVEFLAG_RemoveAllCastling || flags == MOVEFLAG_ShortCastling ||
                             flags == MOVEFLAG_LongCastling) {
            if constexpr (whiteMoved) { cs &= ~wCastleMask; }
            else { cs &= ~bCastleMask; }
        }

        // Promotions
        if constexpr (flags == MOVEFLAG_PromoteQueen) {
            if constexpr (whiteMoved)
                return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks,
                        bRooks & ~to, wQueens | to, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to,
                    bRooks, wQueens & ~to, bQueens | to, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (flags == MOVEFLAG_PromoteRook) {
            if constexpr (whiteMoved)
                return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks | to,
                        bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to,
                    bRooks | to, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (flags == MOVEFLAG_PromoteBishop) {
            if constexpr (whiteMoved)
                return {wPawns & ~from, bPawns, wKnights, bKnights & ~to, wBishops | to, bBishops & ~to, wRooks,
                        bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights, wBishops & ~to, bBishops | to, wRooks & ~to,
                    bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (flags == MOVEFLAG_PromoteKnight) {
            if constexpr (whiteMoved)
                return {wPawns & ~from, bPawns, wKnights | to, bKnights & ~to, wBishops, bBishops & ~to, wRooks,
                        bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns, bPawns & ~from, wKnights & ~to, bKnights | to, wBishops & ~to, bBishops, wRooks & ~to,
                    bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }

        // Castles
        if constexpr (flags == MOVEFLAG_ShortCastling) {
            if constexpr (whiteMoved)
                return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops,
                        wRooks ^ castleShortRookMove<whiteMoved>(), bRooks, wQueens, bQueens,
                        static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0, cs};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks,
                    bRooks ^ castleShortRookMove<whiteMoved>(), wQueens, bQueens, wKingSq,
                    static_cast<uint8_t>(singleBitOf(to)), 0, cs};
        }
        if constexpr (flags == MOVEFLAG_LongCastling) {
            if constexpr (whiteMoved)
                return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops,
                        wRooks ^ castleLongRookMove<whiteMoved>(), bRooks, wQueens, bQueens,
                        static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0, cs};
            return {wPawns, bPawns, wKnights, bKnights, wBishops, bBishops, wRooks,
                    bRooks ^ castleLongRookMove<whiteMoved>(), wQueens, bQueens, wKingSq,
                    static_cast<uint8_t>(singleBitOf(to)), 0, cs};
        }

        // Silent Moves
        if constexpr (piece == PIECE_Pawn) {
            BB epMask = flags == MOVEFLAG_EnPassantCapture ? ~backward<whiteMoved>(newMask(enPassantSq)) : FULL_BB;
            uint8_t epField = flags == MOVEFLAG_PawnDoublePush ? singleBitOf(forward<whiteMoved>(from)) : 0;
            if constexpr (whiteMoved)
                return {wPawns ^ change, bPawns & epMask & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to,
                        wRooks, bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, epField, cs};
            return {wPawns & epMask & ~to, bPawns ^ change, wKnights & ~to, bKnights, wBishops & ~to, bBishops,
                    wRooks & ~to, bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, epField, cs};
        }
        if constexpr (piece == PIECE_Knight) {
            if constexpr (whiteMoved)
                return {wPawns, bPawns & ~to, wKnights ^ change, bKnights & ~to, wBishops, bBishops & ~to, wRooks,
                        bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights ^ change, wBishops & ~to, bBishops, wRooks & ~to,
                    bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == PIECE_Bishop) {
            if constexpr (whiteMoved)
                return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops ^ change, bBishops & ~to, wRooks,
                        bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops ^ change, wRooks & ~to,
                    bRooks, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == PIECE_Rook) {
            if constexpr (whiteMoved)
                return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks ^ change,
                        bRooks & ~to, wQueens, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to,
                    bRooks ^ change, wQueens & ~to, bQueens, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == PIECE_Queen) {
            if constexpr (whiteMoved)
                return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks,
                        bRooks & ~to, wQueens ^ change, bQueens & ~to, wKingSq, bKingSq, 0, cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks,
                    wQueens & ~to, bQueens ^ change, wKingSq, bKingSq, 0, cs};
        }
        if constexpr (piece == PIECE_King) {
            if constexpr (whiteMoved)
                return {wPawns, bPawns & ~to, wKnights, bKnights & ~to, wBishops, bBishops & ~to, wRooks,
                        bRooks & ~to, wQueens, bQueens & ~to, static_cast<uint8_t>(singleBitOf(to)), bKingSq, 0,
                        cs};
            return {wPawns & ~to, bPawns, wKnights & ~to, bKnights, wBishops & ~to, bBishops, wRooks & ~to, bRooks,
                    wQueens & ~to, bQueens, wKingSq, static_cast<uint8_t>(singleBitOf(to)), 0, cs};
        }
//            throw std::exception();
    }

    template<bool whiteToMove>
    constexpr Board Board::fork(const Move &move) const {
        switch (move.piece) {
            case PIECE_Pawn:
                switch (move.flags) {
                    case MOVEFLAG_PawnDoublePush:
                        return fork<whiteToMove, PIECE_Pawn, MOVEFLAG_PawnDoublePush>(newMask(move.fromIndex),
                                                                                      newMask(move.toIndex));
                    case MOVEFLAG_EnPassantCapture:
                        return fork<whiteToMove, PIECE_Pawn, MOVEFLAG_EnPassantCapture>(newMask(move.fromIndex),
                                                                                        newMask(move.toIndex));
                    case MOVEFLAG_PromoteQueen:
                        return fork<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteQueen>(newMask(move.fromIndex),
                                                                                    newMask(move.toIndex));
                    case MOVEFLAG_PromoteRook:
                        return fork<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteRook>(newMask(move.fromIndex),
                                                                                   newMask(move.toIndex));
                    case MOVEFLAG_PromoteBishop:
                        return fork<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteBishop>(newMask(move.fromIndex),
                                                                                     newMask(move.toIndex));
                    case MOVEFLAG_PromoteKnight:
                        return fork<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteKnight>(newMask(move.fromIndex),
                                                                                     newMask(move.toIndex));
                }
                return fork<whiteToMove, PIECE_Pawn>(newMask(move.fromIndex), newMask(move.toIndex));
            case PIECE_Knight:
                return fork<whiteToMove, PIECE_Knight>(newMask(move.fromIndex), newMask(move.toIndex));
            case PIECE_Bishop:
                return fork<whiteToMove, PIECE_Bishop>(newMask(move.fromIndex), newMask(move.toIndex));
            case PIECE_Rook:
                switch (move.flags) {
                    case MOVEFLAG_RemoveShortCastling:
                        return fork<whiteToMove, PIECE_Rook, MOVEFLAG_RemoveShortCastling>(
                                newMask(move.fromIndex), newMask(move.toIndex));
                    case MOVEFLAG_RemoveLongCastling:
                        return fork<whiteToMove, PIECE_Rook, MOVEFLAG_RemoveLongCastling>(newMask(move.fromIndex),
                                                                                          newMask(move.toIndex));
                }
                return fork<whiteToMove, PIECE_Rook>(newMask(move.fromIndex), newMask(move.toIndex));
            case PIECE_Queen:
                return fork<whiteToMove, PIECE_Queen>(newMask(move.fromIndex), newMask(move.toIndex));
            case PIECE_King:
                switch (move.flags) {
                    case MOVEFLAG_RemoveAllCastling:
                        return fork<whiteToMove, PIECE_King, MOVEFLAG_RemoveAllCastling>(newMask(move.fromIndex),
                                                                                         newMask(move.toIndex));
                    case MOVEFLAG_ShortCastling:
                        return fork<whiteToMove, PIECE_King, MOVEFLAG_ShortCastling>(newMask(move.fromIndex),
                                                                                     newMask(move.toIndex));
                    case MOVEFLAG_LongCastling:
                        return fork<whiteToMove, PIECE_King, MOVEFLAG_LongCastling>(newMask(move.fromIndex),
                                                                                    newMask(move.toIndex));
                }
                return fork<whiteToMove, PIECE_King>(newMask(move.fromIndex), newMask(move.toIndex));
            default: __builtin_unreachable();
        }
    }

    template<bool whiteMoved, Piece_t piece, Flag_t flags>
    RestoreInfo Board::makeMove(BB from, BB to) {
        BB change = from | to;
        RestoreInfo ri{enPassantSq, castling, getPieceAt<!whiteMoved>(to)};

        int epSq = enPassantSq;
        enPassantSq = flags == MOVEFLAG_PawnDoublePush ? singleBitOf(forward<whiteMoved>(from)) : 0;

        if constexpr (flags == MOVEFLAG_RemoveShortCastling) {
            if constexpr (whiteMoved) { castling &= ~wCastleShortMask; }
            else { castling &= ~bCastleShortMask; }
        } else if constexpr (flags == MOVEFLAG_RemoveLongCastling) {
            if constexpr (whiteMoved) { castling &= ~wCastleLongMask; }
            else { castling &= ~bCastleLongMask; }
        } else if constexpr (flags == MOVEFLAG_RemoveAllCastling || flags == MOVEFLAG_ShortCastling ||
                             flags == MOVEFLAG_LongCastling) {
            if constexpr (whiteMoved) { castling &= ~wCastleMask; }
            else { castling &= ~bCastleMask; }
        }

        // Promotions
        if constexpr (flags == MOVEFLAG_PromoteQueen) {
            if constexpr (whiteMoved) {
                wPawns &= ~from;
                bKnights &= ~to;
                bBishops &= ~to;
                bRooks &= ~to;
                wQueens |= to;
                bQueens &= ~to;
            }
            else {
                bPawns &= ~from;
                wKnights &= ~to;
                wBishops &= ~to;
                wRooks &= ~to;
                wQueens &= ~to;
                bQueens |= to;
            }
            return ri;
        }
        if constexpr (flags == MOVEFLAG_PromoteRook) {
            if constexpr (whiteMoved) {
                wPawns &= ~from;
                bKnights &= ~to;
                bBishops &= ~to;
                wRooks |= to;
                bRooks &= ~to;
                bQueens &= ~to;
            }
            else {
                bPawns &= ~from;
                wKnights &= ~to;
                wBishops &= ~to;
                wRooks &= ~to;
                bRooks |= to;
                wQueens &= ~to;
            }
            return ri;
        }
        if constexpr (flags == MOVEFLAG_PromoteBishop) {
            if constexpr (whiteMoved) {
                wPawns &= ~from;
                bKnights &= ~to;
                wBishops |= to;
                bBishops &= ~to;
                bRooks &= ~to;
                bQueens &= ~to;
            }
            else {
                bPawns &= ~from;
                wKnights &= ~to;
                wBishops &= ~to;
                bBishops |= to;
                wRooks &= ~to;
                wQueens &= ~to;
            }
            return ri;
        }
        if constexpr (flags == MOVEFLAG_PromoteKnight) {
            if constexpr (whiteMoved) {
                wPawns &= ~from;
                wKnights |= to;
                bKnights &= ~to;
                bBishops &= ~to;
                bRooks &= ~to;
                bQueens &= ~to;
            }
            else {
                bPawns &= ~from;
                wKnights &= ~to;
                bKnights |= to, wBishops &= ~to;
                wRooks &= ~to;
                wQueens &= ~to;
            }
            return ri;
        }

        //Castles
        if constexpr (flags == MOVEFLAG_ShortCastling) {
            if constexpr (whiteMoved) {
                wRooks ^= castleShortRookMove<whiteMoved>();
                wKingSq = static_cast<uint8_t>(singleBitOf(to));
            }
            else {
                bRooks ^= castleShortRookMove<whiteMoved>();
                bKingSq = static_cast<uint8_t>(singleBitOf(to));
            }
            return ri;
        }
        if constexpr (flags == MOVEFLAG_LongCastling) {
            if constexpr (whiteMoved) {
                wRooks ^= castleLongRookMove<whiteMoved>(), wKingSq = static_cast<uint8_t>(singleBitOf(to));
            }
            else { bRooks ^= castleLongRookMove<whiteMoved>(), bKingSq = static_cast<uint8_t>(singleBitOf(to)); }
            return ri;
        }

        // Silent Moves
        if constexpr (piece == PIECE_Pawn) {
            BB epMask = flags == MOVEFLAG_EnPassantCapture ? ~backward<whiteMoved>(newMask(epSq)) : FULL_BB;
            if constexpr (whiteMoved) {
                wPawns ^= change;
                bPawns &= epMask & ~to;
                bKnights &= ~to;
                bBishops &= ~to;
                bRooks &= ~to;
                bQueens &= ~to;
            }
            else {
                wPawns &= epMask & ~to;
                bPawns ^= change;
                wKnights &= ~to;
                wBishops &= ~to;
                wRooks &= ~to;
                wQueens &= ~to;
            }
            return ri;
        }
        if constexpr (piece == PIECE_Knight) {
            if constexpr (whiteMoved) {
                bPawns &= ~to;
                wKnights ^= change;
                bKnights &= ~to;
                bBishops &= ~to;
                bRooks &= ~to;
                bQueens &= ~to;
            }
            else {
                wPawns &= ~to;
                wKnights &= ~to;
                bKnights ^= change;
                wBishops &= ~to;
                wRooks &= ~to;
                wQueens &= ~to;
            }
            return ri;
        }
        if constexpr (piece == PIECE_Bishop) {
            if constexpr (whiteMoved) {
                bPawns &= ~to;
                bKnights &= ~to;
                wBishops ^= change;
                bBishops &= ~to;
                bRooks &= ~to;
                bQueens &= ~to;
            }
            else {
                wPawns &= ~to;
                wKnights &= ~to;
                wBishops &= ~to;
                bBishops ^= change;
                wRooks &= ~to;
                wQueens &= ~to;
            }
            return ri;
        }
        if constexpr (piece == PIECE_Rook) {
            if constexpr (whiteMoved) {
                bPawns &= ~to;
                bKnights &= ~to;
                bBishops &= ~to;
                wRooks ^= change;
                bRooks &= ~to;
                bQueens &= ~to;
            }
            else {
                wPawns &= ~to;
                wKnights &= ~to;
                wBishops &= ~to;
                wRooks &= ~to;
                bRooks ^= change;
                wQueens &= ~to;
            }
            return ri;
        }
        if constexpr (piece == PIECE_Queen) {
            if constexpr (whiteMoved) {
                bPawns &= ~to;
                bKnights &= ~to;
                bBishops &= ~to;
                bRooks &= ~to;
                wQueens ^= change;
                bQueens &= ~to;
            }
            else {
                wPawns &= ~to;
                wKnights &= ~to;
                wBishops &= ~to;
                wRooks &= ~to;
                wQueens &= ~to;
                bQueens ^= change;
            }
            return ri;
        }
        if constexpr (piece == PIECE_King) {
            if constexpr (whiteMoved) {
                bPawns &= ~to;
                bKnights &= ~to;
                bBishops &= ~to;
                bRooks &= ~to;
                bQueens &= ~to, wKingSq = static_cast<uint8_t>(singleBitOf(to));
            }
            else {
                wPawns &= ~to;
                wKnights &= ~to;
                wBishops &= ~to;
                wRooks &= ~to;
                wQueens &= ~to;
                bKingSq = static_cast<uint8_t>(singleBitOf(to));
            }
            return ri;
        }
    }

    template<bool whiteToMove>
    void Board::makeMove(Move move) {
        switch (move.piece) {
            case PIECE_Pawn:
                switch (move.flags) {
                    case MOVEFLAG_PawnDoublePush:
                        makeMove<whiteToMove, PIECE_Pawn, MOVEFLAG_PawnDoublePush>(newMask(move.fromIndex),
                                                                                   newMask(move.toIndex));
                        return;
                    case MOVEFLAG_EnPassantCapture:
                        makeMove<whiteToMove, PIECE_Pawn, MOVEFLAG_EnPassantCapture>(newMask(move.fromIndex),
                                                                                     newMask(move.toIndex));
                        return;
                    case MOVEFLAG_PromoteQueen:
                        makeMove<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteQueen>(newMask(move.fromIndex),
                                                                                 newMask(move.toIndex));
                        return;
                    case MOVEFLAG_PromoteRook:
                        makeMove<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteRook>(newMask(move.fromIndex),
                                                                                newMask(move.toIndex));
                        return;
                    case MOVEFLAG_PromoteBishop:
                        makeMove<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteBishop>(newMask(move.fromIndex),
                                                                                  newMask(move.toIndex));
                        return;
                    case MOVEFLAG_PromoteKnight:
                        makeMove<whiteToMove, PIECE_Pawn, MOVEFLAG_PromoteKnight>(newMask(move.fromIndex),
                                                                                  newMask(move.toIndex));
                        return;
                }
                makeMove<whiteToMove, PIECE_Pawn>(newMask(move.fromIndex), newMask(move.toIndex));
                return;
            case PIECE_Knight:
                makeMove<whiteToMove, PIECE_Knight>(newMask(move.fromIndex), newMask(move.toIndex));
                return;
            case PIECE_Bishop:
                makeMove<whiteToMove, PIECE_Bishop>(newMask(move.fromIndex), newMask(move.toIndex));
                return;
            case PIECE_Rook:
                switch (move.flags) {
                    case MOVEFLAG_RemoveShortCastling:
                        makeMove<whiteToMove, PIECE_Rook, MOVEFLAG_RemoveShortCastling>(newMask(move.fromIndex),
                                                                                        newMask(move.toIndex));
                        return;
                    case MOVEFLAG_RemoveLongCastling:
                        makeMove<whiteToMove, PIECE_Rook, MOVEFLAG_RemoveLongCastling>(newMask(move.fromIndex),
                                                                                       newMask(move.toIndex));
                        return;
                }
                makeMove<whiteToMove, PIECE_Rook>(newMask(move.fromIndex), newMask(move.toIndex));
                return;
            case PIECE_Queen:
                makeMove<whiteToMove, PIECE_Queen>(newMask(move.fromIndex), newMask(move.toIndex));
                return;
            case PIECE_King:
                switch (move.flags) {
                    case MOVEFLAG_RemoveAllCastling:
                        makeMove<whiteToMove, PIECE_King, MOVEFLAG_RemoveAllCastling>(newMask(move.fromIndex),
                                                                                      newMask(move.toIndex));
                        return;
                    case MOVEFLAG_ShortCastling:
                        makeMove<whiteToMove, PIECE_King, MOVEFLAG_ShortCastling>(newMask(move.fromIndex),
                                                                                  newMask(move.toIndex));
                        return;
                    case MOVEFLAG_LongCastling:
                        makeMove<whiteToMove, PIECE_King, MOVEFLAG_LongCastling>(newMask(move.fromIndex),
                                                                                 newMask(move.toIndex));
                        return;
                }
                makeMove<whiteToMove, PIECE_King>(newMask(move.fromIndex), newMask(move.toIndex));
                return;
        }
    }

    template<bool whiteMoved, Piece_t piece, Flag_t flags, Piece_t captured>
    void Board::unmakeMove(BB from, BB to, RestoreInfo ri) {
        BB change = from | to;
        enPassantSq = ri.epSquare;
        castling = ri.castling;

        // Promotions
        if constexpr (flags == MOVEFLAG_PromoteQueen) {
            if constexpr (whiteMoved) {
                wPawns |= from;
                wQueens &= ~to;
            } else {
                bPawns |= from;
                bQueens &= ~to;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
        if constexpr (flags == MOVEFLAG_PromoteRook) {
            if constexpr (whiteMoved) {
                wPawns |= from;
                wRooks &= ~to;
            } else {
                bPawns |= from;
                bRooks &= ~to;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
        if constexpr (flags == MOVEFLAG_PromoteBishop) {
            if constexpr (whiteMoved) {
                wPawns |= from;
                wBishops &= ~to;
            } else {
                bPawns |= from;
                bBishops &= ~to;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
        if constexpr (flags == MOVEFLAG_PromoteKnight) {
            if constexpr (whiteMoved) {
                wPawns |= from;
                wKnights &= ~to;
            } else {
                bPawns |= from;
                bKnights &= ~to;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }

        // Castles
        if constexpr (flags == MOVEFLAG_ShortCastling) {
            constexpr uint8_t startKing = STARTBOARD.kingSquare<whiteMoved>();
            constexpr BB rookMove = castleShortRookMove<whiteMoved>();
            if constexpr (whiteMoved) {
                wKingSq = startKing;
                wRooks ^= rookMove;
            } else {
                bKingSq = startKing;
                bRooks ^= rookMove;
            }
            return;
        }
        if constexpr (flags == MOVEFLAG_LongCastling) {
            constexpr uint8_t startKing = STARTBOARD.kingSquare<whiteMoved>();
            constexpr BB rookMove = castleLongRookMove<whiteMoved>();
            if constexpr (whiteMoved) {
                wKingSq = startKing;
                wRooks ^= rookMove;
            } else {
                bKingSq = startKing;
                bRooks ^= rookMove;
            }
            return;
        }

        // Silent Moves
        if constexpr (piece == PIECE_Pawn) {
            if constexpr (whiteMoved) {
                wPawns ^= change;
            } else {
                bPawns ^= change;
            }
            if constexpr (flags == MOVEFLAG_EnPassantCapture) {
                if constexpr (whiteMoved) {
                    bPawns |= backward<whiteMoved>(newMask(enPassantSq));
                } else {
                    wPawns |= backward<whiteMoved>(newMask(enPassantSq));
                }
            } else {
                restorePiece<!whiteMoved, captured>(to);
            }
            return;
        }

//            constexpr BB pcc = getPieceBB<piece, whiteMoved>();

//            getPieceBB<piece, whiteMoved>() ^= change;
//            restorePiece<!whiteMoved, captured>(to);

        if constexpr (piece == PIECE_Knight) {
            if constexpr (whiteMoved) {
                wKnights ^= change;
            } else {
                bKnights ^= change;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
        if constexpr (piece == PIECE_Bishop) {
            if constexpr (whiteMoved) {
                wBishops ^= change;
            } else {
                bBishops ^= change;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
        if constexpr (piece == PIECE_Rook) {
            if constexpr (whiteMoved) {
                wRooks ^= change;
            } else {
                bRooks ^= change;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
        if constexpr (piece == PIECE_Queen) {
            if constexpr (whiteMoved) {
                wQueens ^= change;
            } else {
                bQueens ^= change;
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
        if constexpr (piece == PIECE_King) {
            if constexpr (whiteMoved) {
                wKingSq = static_cast<uint8_t>(singleBitOf(from));
            } else {
                bKingSq = static_cast<uint8_t>(singleBitOf(from));
            }
            restorePiece<!whiteMoved, captured>(to);
            return;
        }
//        throw std::exception();
    }

    template<bool whiteMoved, Piece_t piece, Flag_t flags>
    void Board::unmakeMove(BB from, BB to, RestoreInfo ri) {
        switch (ri.captured) {
            case PIECE_Pawn:    unmakeMove<whiteMoved, piece, flags, PIECE_Pawn>(from, to, ri); break;
            case PIECE_Knight:  unmakeMove<whiteMoved, piece, flags, PIECE_Knight>(from, to, ri); break;
            case PIECE_Bishop:  unmakeMove<whiteMoved, piece, flags, PIECE_Bishop>(from, to, ri); break;
            case PIECE_Rook:    unmakeMove<whiteMoved, piece, flags, PIECE_Rook>(from, to, ri); break;
            case PIECE_Queen:   unmakeMove<whiteMoved, piece, flags, PIECE_Queen>(from, to, ri); break;
            default: unmakeMove<whiteMoved, piece, flags, PIECE_None>(from, to, ri); break;
        }
    }

} // namespace Dory

#endif //DORY_BOARD_H
