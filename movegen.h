//
// Created by Robin on 01.07.2022.
//

#include <cstdint>
#include "checklogichandler.h"
#include "utils.h"

#ifndef CHESSENGINE_MOVEGEN_H
#define CHESSENGINE_MOVEGEN_H

class MoveList {
    int i{0};

    void add(Move m) {
        moves[i++] = m;
    }

public:
    void add(uint8_t piece, BB from, BB to, uint8_t flags) {
        if(i >= 256) throw std::exception();
        moves[i].from = from;
        moves[i].to = to;
        moves[i].piece = piece;
        moves[i].flags = flags;
        i++;
    }

    void add(uint8_t piece, BB from, BB to) {
        add(piece, from, to, MoveFlag::Silent);
    }

    void reset() {
        i = 0;
    }

    [[nodiscard]] constexpr int size() const {
        return i;
    }

    std::array<Move, 256> moves{};
};

class MoveGenerator {
    CheckLogicHandler clh{};
    MoveList lst{};
    BB checkMask{0}, targetSquares{0}, pinsStr{0}, pinsDiag{0};

public:
    template<State state>
    const MoveList* generate(Board& board) {
        clh.reload<state>(board);
        lst.reset();

        checkMask = clh.getCheckMask();
        targetSquares = board.enemyOrEmpty<state.whiteToMove>() & checkMask;
        pinsDiag = clh.pinsDiagonal;
        pinsStr = clh.pinsStraight;

        if(!clh.isDoubleCheck) {
            pawnMoves<state>(board);
            knightMoves<state>(board);
            bishopMoves<state>(board);
            rookMoves<state>(board);
            queenMoves<state>(board);
            castles<state>(board);
        }

        kingMoves<state>(board);

        return &lst;
    }

private:
    template<State state>
    [[nodiscard]] constexpr uint8_t getMoveFlag(uint8_t piece, BB from) const {
        if(state.canCastleShort()
           && piece == Piece::Rook
           && startingKingsideRook<state.whiteToMove>() & from
        ) return MoveFlag::RemoveShortCastling;

        if(state.canCastleLong()
           && piece == Piece::Rook
           && startingQueensideRook<state.whiteToMove>() & from
        ) return MoveFlag::RemoveLongCastling;

        if(piece == Piece::King) return MoveFlag::RemoveAllCastling;

        return MoveFlag::Silent;
    }

    template<State state>
    void addToList(uint8_t piece, int from, BB targets) {
        while(targets != 0) {
            int ix = lastBitOf(targets);
            deleteBitAt(targets, ix);
            BB fromBB = newMask(from);
            BB toBB = newMask(ix);
            lst.add(
                piece, fromBB, toBB,
                getMoveFlag<state>(piece, fromBB)
            );
        }
    }

    void handlePromotions(BB from, BB to) {
        lst.add(Piece::Pawn, from, to, MoveFlag::PromoteQueen);
        lst.add(Piece::Pawn, from, to, MoveFlag::PromoteRook);
        lst.add(Piece::Pawn, from, to, MoveFlag::PromoteBishop);
        lst.add(Piece::Pawn, from, to, MoveFlag::PromoteKnight);
    }

    template<State state>
    void pawnMoves(Board& board) {
        constexpr bool white = state.whiteToMove;
        BB free = board.free();
        BB enemy = board.enemyPieces<white>();
        BB pawnsFwd = board.pawns<white>() & ~pinsDiag;
        BB pawnCapt = board.pawns<white>() & ~pinsStr;

        // pawns that can move 1 or 2 squares
        BB pwnMov = pawnsFwd & backward<white>(free);
        BB pwnMov2 = pwnMov & backward2<white>(free & checkMask) & firstRank<white>();
        pwnMov &= backward<white>(checkMask);

        // pawns that can capture Left or Right
        BB pawnCapL = pawnCapt & pawnInvAtkLeft<white>(enemy & checkMask) & pawnCanGoLeft<white>();
        BB pawnCapR = pawnCapt & pawnInvAtkRight<white>(enemy & checkMask) & pawnCanGoRight<white>();

        // remove pinned pawns
        pwnMov      &= backward<white> (pinsStr) | ~pinsStr;
        pwnMov2     &= backward2<white>(pinsStr) | ~pinsStr;
        pawnCapL    &= pawnInvAtkLeft<white> (pinsDiag & pawnCanGoRight<white>()) | ~pinsDiag;
        pawnCapR    &= pawnInvAtkRight<white>(pinsDiag & pawnCanGoLeft <white>()) | ~pinsDiag;

        // handle en passant pawns
        BB epPawnL = 0, epPawnR = 0;
        constexpr BB enPassant = state.enPassantField;
        if(enPassant != 0) {
            // left capture is ep square and is on checkmask
            epPawnL = pawnCapt & pawnCanGoLeft<white>() & pawnInvAtkLeft<white>(enPassant & forward<white>(checkMask));
            // remove pinned ep pawns
            epPawnL &= pawnInvAtkLeft<white>(pinsDiag & pawnCanGoLeft<white>()) | ~pinsDiag;
            // handle very special case of two sideways pinned epPawns
            epPawnL = clh.pruneEpPin(epPawnL);

            // right capture is ep square and is on checkmask
            epPawnR = pawnCapt & pawnCanGoRight<white>() & pawnInvAtkRight<white>(enPassant & forward<white>(checkMask));
            // remove pinned ep pawns
            epPawnR &= pawnInvAtkRight<white>(pinsDiag & pawnCanGoRight<white>()) | ~pinsDiag;
            // handle very special case of two sideways pinned epPawns
            epPawnR = clh.pruneEpPin(epPawnR);
        }

        // collect all promoting pawns in separate variables
        BB lastRowMask = pawnOnLastRow<white>();
        BB pwnPromoteFwd  = pwnMov   & lastRowMask;
        BB pwnPromoteL    = pawnCapL & lastRowMask;
        BB pwnPromoteR    = pawnCapR & lastRowMask;

        // remove all promoting pawns from these collections
        pwnMov &= ~lastRowMask;
        pawnCapL &= ~lastRowMask;
        pawnCapR &= ~lastRowMask;

        BB fromBB = 1;
        for(int i = 0; i < 64; i++) {
            // non-promoting pawn moves
            if((pwnMov & fromBB) != 0) {    // straight push, 1 square
                lst.add(Piece::Pawn, fromBB, forward<white>(fromBB));
            }
            if((pawnCapL & fromBB) != 0) {  // capture towards left
                lst.add(Piece::Pawn, fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((pawnCapR & fromBB) != 0) {  // capture towards right
                lst.add(Piece::Pawn, fromBB, pawnAtkRight<white>(fromBB));
            }

            // promoting pawn moves
            if((pwnPromoteFwd & fromBB) != 0) {  // single push + promotion
                handlePromotions(fromBB, forward<white>(fromBB));
            }
            if((pwnPromoteL & fromBB) != 0) {  // capture left + promotion
                handlePromotions(fromBB, pawnAtkLeft<white>(fromBB));
            }
            if((pwnPromoteR & fromBB) != 0) {  // capture right + promotion
                handlePromotions(fromBB, pawnAtkRight<white>(fromBB));
            }

            // pawn moves that cannot be promotions
            if((pwnMov2 & fromBB) != 0) {   // pawn double push
                lst.add(Piece::Pawn, fromBB, forward2<white>(fromBB), MoveFlag::PawnDoublePush);
            }
            if((epPawnL & fromBB) != 0) {   // pawn ep capture towards left
                lst.add(Piece::Pawn, fromBB, pawnAtkLeft<white>(fromBB), MoveFlag::EnPassantCapture);
            }
            if((epPawnR & fromBB) != 0) {   // pawn ep capture towards right
                lst.add(Piece::Pawn, fromBB, pawnAtkRight<white>(fromBB), MoveFlag::EnPassantCapture);
            }

            fromBB <<= 1;
        }
    }


    template<State state>
    void knightMoves(Board& board) {
        BB movKnights = board.knights<state.whiteToMove>() & ~clh.allPins();

        while(movKnights != 0) {
            int ix = lastBitOf(movKnights);
            deleteBitAt(movKnights, ix);

            BB targets = PieceSteps::KNIGHT_MOVES[ix] & targetSquares;
            addToList<state>(Piece::Knight, ix, targets);
        }
    }

    template<State state>
    void bishopMoves(Board& board) {
        BB bishops = board.bishops<state.whiteToMove>() & ~pinsStr;

        while(bishops != 0) {
            int ix = lastBitOf(bishops);
            deleteBitAt(bishops, ix);

            BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, ix) & targetSquares;
            if(hasBitAt(pinsDiag, ix)) targets &= pinsDiag;
            addToList<state>(Piece::Bishop, ix, targets);
        }
    }

    template<State state>
    void rookMoves(Board& board) {
        BB rooks = board.rooks<state.whiteToMove>() & ~pinsDiag;

        while(rooks != 0) {
            int ix = lastBitOf(rooks);
            deleteBitAt(rooks, ix);

            BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, ix) & targetSquares;
            if(hasBitAt(pinsStr, ix)) targets &= pinsStr;
            addToList<state>(Piece::Rook, ix, targets);
        }
    }

    template<State state>
    void queenMoves(Board& board) {
        BB queens = board.queens<state.whiteToMove>();
        BB queensPinStr = queens & pinsStr & ~pinsDiag;
        BB queensPinDiag = queens & pinsDiag & ~pinsStr;
        BB queensNoPin = queens & ~(pinsDiag | pinsStr);

        BB selector = 1;
        for(int i = 0; i < 64; i++) {
            if((queensPinStr & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, i) & targetSquares & pinsStr;
                addToList<state>(Piece::Queen, i, targets);
            }
            else if((queensPinDiag & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, true, false>(board, i) & targetSquares & pinsDiag;
                addToList<state>(Piece::Queen, i, targets);
            }
            else if((queensNoPin & selector) != 0) {
                BB targets = PieceSteps::slideMask<state.whiteToMove, false, false>(board, i) & targetSquares;
                targets |= PieceSteps::slideMask<state.whiteToMove, true, false>(board, i) & targetSquares;
                addToList<state>(Piece::Queen, i, targets);
            }
            selector <<= 1;
        }
    }

    template<State state>
    void kingMoves(Board& board) {
        BB king = board.king<state.whiteToMove>();
        int ix = singleBitOf(king);
        BB targets = PieceSteps::KING_MOVES[ix] & ~clh.attacked & board.enemyOrEmpty<state.whiteToMove>();
        addToList<state>(Piece::King, ix, targets);
    }

    template<State state>
    void castles(Board& board) {
        constexpr bool white = state.whiteToMove;
        BB kingBB = board.king<white>();
        BB startKing = white ? STARTBOARD.wKing : STARTBOARD.bKing;
        BB csMask = castleShortMask<white>();
        BB clMask = castleLongMask<white>();

        if(kingBB == startKing) {
            if(state.canCastleShort()
               && board.rooks<white>() & startingKingsideRook<white>()
               && csMask & ~clh.attacked
               && (csMask & board.occ()) == kingBB
            ) lst.add(Piece::King, kingBB, kingBB << 2, MoveFlag::ShortCastling);

            if(state.canCastleLong()
                && board.rooks<white>() & startingQueensideRook<white>()
                && clMask & ~clh.attacked
                && (clMask & board.occ()) == kingBB
                && board.free() & (startingQueensideRook<white>() << 1)
            ) lst.add(Piece::King, kingBB, kingBB >> 2, MoveFlag::LongCastling);
        }

    }
};

#endif //CHESSENGINE_MOVEGEN_H
