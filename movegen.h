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
    std::array<Move, 256> moves{};
    int i{0};

    void add(Move m) {
        moves[i++] = m;
    }

    void add(BB from, BB to, uint8_t piece, uint8_t flags) {
        moves[i].from = from;
        moves[i].to = to;
        moves[i].piece = piece;
        moves[i].flags = flags;
        i++;
    }

};

class MoveGenerator {
    const CheckLogicHandler clh;
    const MoveList lst{};
    BB checkMask{0}, targetSquares{0};

public:
    explicit MoveGenerator(CheckLogicHandler c) : clh{c} {}

    template<State state>
    const MoveList* generate(Board& board) {
        checkMask = clh.getCheckMask();
        targetSquares = board.enemyOrEmpty<state.whiteToMove>() & checkMask;

        if(!clh.isDoubleCheck) {
//            pawnMoves();
            knightMoves<state>(board);
//            bishopMoves();
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
                piece, fromBB, toBB,
                getMoveFlag<state>(piece, from, ix)
            );
        }
    }


    template<State state>
    void knightMoves(Board& board) {
        BB movKnights = board.knights<state.whiteToMove>() & ~clh.allPins();

        while(movKnights != 0) {
            int ix = firstBitOf(movKnights);
            deleteBitAt(movKnights, ix);

            BB targets = PieceSteps::KNIGHT_MOVES[ix] & targetSquares;
            addToList<state>(Piece::Knight, ix, targets);
        }
    }
};

#endif //CHESSENGINE_MOVEGEN_H
