//
// Created by robin on 24.08.2023.
//

#ifndef DORY_FEATURES_H
#define DORY_FEATURES_H

#include "../board.h"
#include "engine_params.h"

using Params = engine_params::EvaluationParams;

namespace features {

    template<bool whiteToMove>
    int material(const Board& board, Params params) {
        return bitCount(board.pawns<whiteToMove>()) * params.MATERIAL_WEIGHT_PAWN +
                bitCount(board.knights<whiteToMove>()) * params.MATERIAL_WEIGHT_KNIGHT +
                bitCount(board.bishops<whiteToMove>()) * params.MATERIAL_WEIGHT_BISHOP +
                bitCount(board.rooks<whiteToMove>()) * params.MATERIAL_WEIGHT_ROOK +
                bitCount(board.queens<whiteToMove>()) * params.MATERIAL_WEIGHT_QUEEN;
    }

}

#endif //DORY_FEATURES_H
