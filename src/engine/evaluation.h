//
// Created by robin on 24.08.2023.
//

#ifndef DORY_EVALUATION_H
#define DORY_EVALUATION_H

#include <iostream>
#include <stack>
#include <vector>
#include <memory>
#include <thread>
#include <unordered_map>

#include "features.h"
#include "engine_params.h"
#include "engine.h"
#include "../movecollectors.h"
#include "../movegen.h"

namespace evaluation {
    static engine_params::EvaluationParams params{};

    /**
     * Gives estimate for position evaluation score for the side that is to move.
     * Positive value is good for the side to move (not necessarily good for white)
     */
    int evaluatePosition(const Board& board, const State state) {

        int material = features::material<true>(board, params) - features::material<false>(board, params);

        int mobility = features::mobility<true>(board, params) - features::mobility<false>(board, params);

        int activity = features::activity<true>(board, params) - features::activity<false>(board, params);

        int evalEstimate = material * params.MATERIAL_QUANTIFIER
                + mobility * params.MOBILITY_QUANTIFIER
                + activity * params.ACTIVITY_QUANTIFIER;

        return state.whiteToMove ? evalEstimate : -evalEstimate;
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
        if(priorityMove.is<piece, flags>(from, to)) {
            return 999999;
        }

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
//        int position_eval_diff = position_evaluate(board) - evaluatePosition(nextBoard);
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
