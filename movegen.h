//
// Created by Robin on 01.07.2022.
//

#include <cstdint>
#include "checklogichandler.h"

#ifndef CHESSENGINE_MOVEGEN_H
#define CHESSENGINE_MOVEGEN_H

struct Move {
    BB from{0}, to{0};
    uint8_t piece{0}, flags{0};
};

class MoveList {
    int i{0};

    void add(Move m) {
        moves[i++] = m;
    }

public:
    void add(BB from, BB to, uint8_t piece, uint8_t flags) {
        moves[i].from = from;
        moves[i].to = to;
        moves[i].piece = piece;
        moves[i].flags = flags;
        i++;
    }

    std::array<Move, 256> moves{};
};

class MoveGenerator {
    const CheckLogicHandler clh;
    MoveList lst{};
    BB checkMask{0}, targetSquares{0};

public:
    explicit MoveGenerator(CheckLogicHandler clh) : clh{clh} {}

    template<State state, Board board>
    const MoveList* generate() {
        checkMask = clh.getCheckMask();
        targetSquares = board.enemyOrEmpty<state.whiteToMove>() & checkMask;

        if(!clh.isDoubleCheck) {
//            pawnMoves();
            knightMoves<state, board>();
            bishopMoves<state, board>();
//            rookMoves();
//            queenMoves();
//            castles();
        }

//        kingMoves();

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
            int ix = firstBitOf(targets);
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
            int ix = firstBitOf(movKnights);
            deleteBitAt(movKnights, ix);

            BB targets = PieceSteps::KNIGHT_MOVES[ix] & targetSquares;
            addToList<state>(Piece::Knight, ix, targets);
        }
    }

    template<State state, Board board>
    void bishopMoves() {
        BB bishops = board.bishops<state.whiteToMove>() & ~clh.pinsStraight;

        while(bishops != 0) {
            int ix = firstBitOf(bishops);
            deleteBitAt(bishops, ix);

            BB targets = PieceSteps::slideMask<board, state.whiteToMove, true, false>(ix) & targetSquares;
            if(hasBitAt(clh.pinsDiagonal, ix)) targets &= clh.pinsDiagonal;
            addToList<state>(Piece::Bishop, ix, targets);
        }
    }

    template<State state, Board board>
    void rookMoves() {
        BB rooks = board.rooks<state.whiteToMove>() & ~clh.pinsDiagonal;

        while(rooks != 0) {
            int ix = firstBitOf(rooks);
            deleteBitAt(rooks, ix);

            BB targets = PieceSteps::slideMask<board, state.whiteToMove, false, false>(ix) & targetSquares;
            if(hasBitAt(clh.pinsStraight, ix)) targets &= clh.pinsStraight;
            addToList<state>(Piece::Rook, ix, targets);
        }
    }
};

#endif //CHESSENGINE_MOVEGEN_H
