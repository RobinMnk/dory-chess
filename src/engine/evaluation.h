//
// Created by robin on 24.08.2023.
//

#ifndef DORY_EVALUATION_H
#define DORY_EVALUATION_H

#include "features.h"
#include "engine_params.h"
#include "engine.h"
#include "../movecollectors.h"
#include "../movegen.h"

namespace evaluation {

    int position_evaluate(const Board& board) {
        engine_params::EvaluationParams params;
        return features::material(board, params);
    }

    template<State state>
    static int isForwardMove(BB from, BB to) {
        if constexpr (state.whiteToMove) {
            return (singleBitOf(to) - singleBitOf(from)) / 8;
        } else {
            return (singleBitOf(from) - singleBitOf(to)) / 8;
        }
    }

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static int move_heuristic(const Board &board, BB from, BB to, PDptr& pd, Move priorityMove) {
        if(priorityMove.is(from, to, piece, flags)) {
            return 999999;
        }

        engine_params::EvaluationParams params;

        int heuristic_val{0};

        // is Capture
        if ((to & board.enemyPieces<state.whiteToMove>()) != 0) {
            int valueDiff = -engine_params::pieceValue<piece>(params);
            if (board.enemyPawns<state.whiteToMove>() & to) {
                valueDiff += engine_params::pieceValue<Piece::Pawn>(params);
            }
            if (board.enemyKnights<state.whiteToMove>() & to) {
                valueDiff += engine_params::pieceValue<Piece::Knight>(params);
            }
            if (board.enemyBishops<state.whiteToMove>() & to) {
                valueDiff += engine_params::pieceValue<Piece::Bishop>(params);
            }
            if (board.enemyRooks<state.whiteToMove>() & to) {
                valueDiff += engine_params::pieceValue<Piece::Rook>(params);
            }
            if (board.enemyQueens<state.whiteToMove>() & to) {
                valueDiff += engine_params::pieceValue<Piece::Queen>(params);
            }

            heuristic_val += 1000 + valueDiff;
        }

        if constexpr (flags == MoveFlag::PromoteBishop) {
            heuristic_val += 1300;
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            heuristic_val += 1300;
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            heuristic_val += 1500;
        }
        if constexpr (flags == MoveFlag::PromoteQueen) {
            heuristic_val += 2000;
        }

//        Board nextBoard = board.getNextBoard<state, piece, flags>(from, to);
//        int position_eval_diff = position_evaluate(board) - position_evaluate(nextBoard);
//        heuristic_val += position_eval_diff;

        heuristic_val += pieceValue<piece>(params) / 200;

//        if constexpr (piece != Piece::Pawn) {
            if (to & pd->pawnAtk) {
                heuristic_val -= pieceValue<piece>(params) * 2;
            } else if (to & pd->attacked) {
                heuristic_val -= pieceValue<piece>(params);
            }
//        }

//        heuristic_val += isForwardMove<state>(from, to) / 4;

//        if(to & pd->attacked) {
//            heuristic_val -= pieceValue<piece>(params) / 900;
//        }



        return heuristic_val;
    }
}


#endif //DORY_EVALUATION_H
