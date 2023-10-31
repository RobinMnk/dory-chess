//
// Created by robin on 24.08.2023.
//

#ifndef DORY_FEATURES_H
#define DORY_FEATURES_H

#include "../board.h"
#include "engine_params.h"

using Params = engine_params::EvaluationParams;

namespace features {

    int material(const Board& board, Params params) {
        int mat = (bitCount(board.wPawns) - bitCount(board.bPawns)) * params.MATERIAL_WEIGHT_PAWN +
                (bitCount(board.wKnights) - bitCount(board.bKnights)) * params.MATERIAL_WEIGHT_KNIGHT +
                (bitCount(board.wBishops) - bitCount(board.bBishops)) * params.MATERIAL_WEIGHT_BISHOP +
                (bitCount(board.wRooks) - bitCount(board.bRooks)) * params.MATERIAL_WEIGHT_ROOK +
                (bitCount(board.wQueens) - bitCount(board.bQueens)) * params.MATERIAL_WEIGHT_QUEEN;

        return mat * params.MATERIAL_QUANTIFIER;
    }

}

#endif //DORY_FEATURES_H
