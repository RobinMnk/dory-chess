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
    void add(BB from, BB to, uint8_t piece, uint8_t flags) {
        if(i >= 256) throw std::exception();
        moves[i].from = from;
        moves[i].to = to;
        moves[i].piece = piece;
        moves[i].flags = flags;
        i++;
    }

    void reset() {
        i = 0;
    }

    std::array<Move, 256> moves{};
};

class MoveGenerator {
    const CheckLogicHandler clh;
    MoveList lst{};
    BB checkMask{0}, targetSquares{0}, pinsStr{0}, pinsDiag{0};

public:
    explicit MoveGenerator(CheckLogicHandler clh) : clh{clh} {}

    template<State state, Board board>
    const MoveList* generate() {
        checkMask = clh.getCheckMask();
        targetSquares = board.enemyOrEmpty<state.whiteToMove>() & checkMask;
        pinsDiag = clh.pinsDiagonal;
        pinsStr = clh.pinsStraight;

        if(!clh.isDoubleCheck) {
//            pawnMoves();
            knightMoves<state, board>();
            bishopMoves<state, board>();
            rookMoves<state, board>();
            queenMoves<state, board>();
//            castles();
        }

        kingMoves<state, board>();

        return &lst;
    }

private:
    template<State state>
    [[nodiscard]] constexpr uint8_t getMoveFlag(uint8_t piece, int from) const {
        if(state.canCastleShort()
           && piece == Piece::Rook
           && from == startingPosKingsideRook<state.whiteToMove>()
        ) return MoveFlag::RemoveShortCastling;

        if(state.canCastleLong()
           && piece == Piece::Rook
           && from == startingPosQueensideRook<state.whiteToMove>()
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
                fromBB, toBB, piece,
                getMoveFlag<state>(piece, from)
            );
        }
    }


    template<State state, Board board>
    void knightMoves() {
        BB movKnights = board.knights<state.whiteToMove>() & ~clh.allPins();

        while(movKnights != 0) {
            int ix = lastBitOf(movKnights);
            deleteBitAt(movKnights, ix);

            BB targets = PieceSteps::KNIGHT_MOVES[ix] & targetSquares;
            addToList<state>(Piece::Knight, ix, targets);
        }
    }

    template<State state, Board board>
    void bishopMoves() {
        BB bishops = board.bishops<state.whiteToMove>() & ~pinsStr;

        while(bishops != 0) {
            int ix = lastBitOf(bishops);
            deleteBitAt(bishops, ix);

            BB targets = PieceSteps::slideMask<board, state.whiteToMove, true, false>(ix) & targetSquares;
            if(hasBitAt(pinsDiag, ix)) targets &= pinsDiag;
            addToList<state>(Piece::Bishop, ix, targets);
        }
    }

    template<State state, Board board>
    void rookMoves() {
        BB rooks = board.rooks<state.whiteToMove>() & ~pinsDiag;

        while(rooks != 0) {
            int ix = lastBitOf(rooks);
            deleteBitAt(rooks, ix);

            BB targets = PieceSteps::slideMask<board, state.whiteToMove, false, false>(ix) & targetSquares;
            if(hasBitAt(pinsStr, ix)) targets &= pinsStr;
            addToList<state>(Piece::Rook, ix, targets);
        }
    }

    template<State state, Board board>
    void queenMoves() {
        BB queens = board.queens<state.whiteToMove>();
        BB queensPinStr = queens & pinsStr & ~pinsDiag;
        BB queensPinDiag = queens & pinsDiag & ~pinsStr;
        BB queensNoPin = queens & ~(pinsDiag | pinsStr);

        BB selector = 1;
        for(int i = 0; i < 64; i++) {
            if((queensPinStr & selector) != 0) {
                BB targets = PieceSteps::slideMask<board, state.whiteToMove, false, false>(i) & targetSquares & pinsStr;
                addToList<state>(Piece::Queen, i, targets);
            }
            else if((queensPinDiag & selector) != 0) {
                BB targets = PieceSteps::slideMask<board, state.whiteToMove, true, false>(i) & targetSquares & pinsDiag;
                addToList<state>(Piece::Queen, i, targets);
            }
            else if((queensNoPin & selector) != 0) {
                BB targets = PieceSteps::slideMask<board, state.whiteToMove, false, false>(i) & targetSquares;
                targets |= PieceSteps::slideMask<board, state.whiteToMove, true, false>(i) & targetSquares;
                addToList<state>(Piece::Queen, i, targets);
            }
            selector <<= 1;
        }
    }

    template<State state, Board board>
    void kingMoves() {
        long king = board.king<state.whiteToMove>();
        int ix = singleBitOf(king);
        long targets = PieceSteps::KING_MOVES[ix] & ~clh.attacked & board.enemyOrEmpty<state.whiteToMove>();
        addToList<state>(Piece::King, ix, targets);
    }
};

#endif //CHESSENGINE_MOVEGEN_H
