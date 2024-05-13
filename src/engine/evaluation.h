//
// Created by Robin on 24.08.2023.
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
#include "../core/movecollectors.h"

namespace evaluation {
    static engine_params::EvaluationParams params{};

    /**
     * Gives estimate for position evaluation score for the side that is to move.
     * Positive value is good for the side to move (not necessarily good for white)
     */
    int evaluatePosition(const Board& board, const State state) {

//        int material = features::material<true>(board, params) - features::material<false>(board, params);
//
//        int mobility = features::mobility<true>(board, params) - features::mobility<false>(board, params);
//
        int activity = features::activity<true>(board, params) - features::activity<false>(board, params);

//        int evalEstimate = material * params.MATERIAL_QUANTIFIER
////                + mobility * params.MOBILITY_QUANTIFIER
//                + activity * params.ACTIVITY_QUANTIFIER;
//
//        evalEstimate /= 2; // seems important, not sure why -> because of the aspiration window!

        int evalEstimate = activity;

        return state.whiteToMove ? evalEstimate : -evalEstimate;

//        return state.whiteToMove ? features::activity<true>(board, params) : features::activity<false>(board, params);
    }

    template<State state>
    static int isForwardMove(BB from, BB to) {
        if constexpr (state.whiteToMove) {
            return (singleBitOf(to) - singleBitOf(from)) / 8;
        } else {
            return (singleBitOf(from) - singleBitOf(to)) / 8;
        }
    }

    const int Large = 1000000;

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static int move_heuristic(const Board &board, BB from, BB to, PDptr& pd, Move priorityMove) {
        if(priorityMove.is<piece, flags>(from, to)) {
            return INF;
        }

        int heuristic_val{0};

        /// Captures
        if ((to & board.enemyPieces<state.whiteToMove>())) {
            int valueDiff = -engine_params::pieceValue<piece>(params);

            if (board.enemyPawns<state.whiteToMove>() & to)
                valueDiff += engine_params::pieceValue<Piece::Pawn>(params);
            else if (board.enemyKnights<state.whiteToMove>() & to)
                valueDiff += engine_params::pieceValue<Piece::Knight>(params);
            else if (board.enemyBishops<state.whiteToMove>() & to)
                valueDiff += engine_params::pieceValue<Piece::Bishop>(params);
            else if (board.enemyRooks<state.whiteToMove>() & to)
                valueDiff += engine_params::pieceValue<Piece::Rook>(params);
            else if (board.enemyQueens<state.whiteToMove>() & to)
                valueDiff += engine_params::pieceValue<Piece::Queen>(params);

            heuristic_val += 2 * Large + valueDiff;

            if((pd->attacked & to) && valueDiff >= 0) {
                // Opponent can recapture
                heuristic_val += 8 * Large;
            }
        }

        /// Checks
//        if (pd->inCheck()) {
//            heuristic_val += 10 * Large + 100;
//        }

        /// Promotions
        if constexpr (flags == MoveFlag::PromoteBishop) {
            heuristic_val += 3 * Large + 3200;
        }
        if constexpr (flags == MoveFlag::PromoteKnight) {
            heuristic_val += 3 * Large + 3000;
        }
        if constexpr (flags == MoveFlag::PromoteRook) {
            heuristic_val += 3 * Large + 5000;
        }
        if constexpr (flags == MoveFlag::PromoteQueen) {
            heuristic_val += 7 * Large;
        }

//        heuristic_val += pieceValue<piece>(params) / 200;

        /// Do not move to an attacked square
        if (to & pd->pawnAtk) {
            heuristic_val -= 50; // pieceValue<piece>(params) * 2;
        } else if (to & pd->attacked) {
            heuristic_val -= 25; // pieceValue<piece>(params);
        }

        heuristic_val += isForwardMove<state>(from, to) / 4;

//        if(to & pd->attacked) {
//            heuristic_val -= pieceValue<piece>(params) / 1024;
//        }

        /// Activity difference
        int activity_diff_mg = params.middleGamePieceTable<piece, state.whiteToMove>(firstBitOf(to))
                - params.middleGamePieceTable<piece, state.whiteToMove>(firstBitOf(from));
        int activity_diff_eg = params.endGamePieceTable<piece, state.whiteToMove>(firstBitOf(to))
                               - params.endGamePieceTable<piece, state.whiteToMove>(firstBitOf(from));

        heuristic_val += activity_diff_mg + activity_diff_eg;

        return heuristic_val;
    }
}


#endif //DORY_EVALUATION_H
