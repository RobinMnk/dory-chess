//
// Created by robin on 24.08.2023.
//

#ifndef DORY_FEATURES_H
#define DORY_FEATURES_H

#include "../board.h"
#include "engine_params.h"

using Params = engine_params::EvaluationParams;

namespace features {

    float material(const BoardPtr& board, Params params) {
        float mat = static_cast<float>(bitCount(board->wPawns) - bitCount(board->bPawns)) * params.MATERIAL_WEIGHT_PAWN +
                static_cast<float>(bitCount(board->wKnights) - bitCount(board->bKnights)) * params.MATERIAL_WEIGHT_KNIGHT +
                static_cast<float>(bitCount(board->wBishops) - bitCount(board->bBishops)) * params.MATERIAL_WEIGHT_BISHOP +
                static_cast<float>(bitCount(board->wRooks) - bitCount(board->bRooks)) * params.MATERIAL_WEIGHT_ROOK +
                static_cast<float>(bitCount(board->wQueens) - bitCount(board->bQueens)) * params.MATERIAL_WEIGHT_QUEEN;

        return mat * params.MATERIAL_QUANTIFIER;
    }

}

#endif //DORY_FEATURES_H
