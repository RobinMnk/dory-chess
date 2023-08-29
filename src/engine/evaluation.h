//
// Created by robin on 24.08.2023.
//

#ifndef DORY_EVALUATION_H
#define DORY_EVALUATION_H

#include "features.h"
#include "engine_params.h"

namespace evaluation {

    double position_evaluate(const Board& board) {
        engine_params::EvaluationParams params;
        return features::material(board, params);
    }

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static double move_heuristic(const Board &board, BB from, BB to) {
        engine_params::EvaluationParams params;

        double heuristic_val{0};

        // is Capture
        if ((to & board.enemyPieces<state.whiteToMove>()) != 0) {
            double valueDiff = engine_params::pieceValue<piece>(params);
            if (board.enemyPawns<state.whiteToMove>() & to) {
                valueDiff -= engine_params::pieceValue<Piece::Pawn>(params);
            }
            if (board.enemyKnights<state.whiteToMove>() & to) {
                valueDiff -= engine_params::pieceValue<Piece::Knight>(params);
            }
            if (board.enemyBishops<state.whiteToMove>() & to) {
                valueDiff -= engine_params::pieceValue<Piece::Bishop>(params);
            }
            if (board.enemyRooks<state.whiteToMove>() & to) {
                valueDiff -= engine_params::pieceValue<Piece::Rook>(params);
            }
            if (board.enemyQueens<state.whiteToMove>() & to) {
                valueDiff -= engine_params::pieceValue<Piece::Queen>(params);
            }

            heuristic_val += 1 + valueDiff * 2;
        }

        Board nextBoard = board.getNextBoard<state, piece, flags>(from, to);
        double position_eval_diff = position_evaluate(board) - position_evaluate(nextBoard);

        heuristic_val += position_eval_diff;

        return heuristic_val;
    }




};


#endif //DORY_EVALUATION_H
