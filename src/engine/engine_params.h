//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_PARAMS_H
#define DORY_ENGINE_PARAMS_H

namespace engine_params {

    struct EvaluationParams {
        double MATERIAL_WEIGHT_PAWN = 1.;
        double MATERIAL_WEIGHT_KNIGHT = 3.;
        double MATERIAL_WEIGHT_BISHOP = 3.1;
        double MATERIAL_WEIGHT_ROOK = 5.;
        double MATERIAL_WEIGHT_QUEEN = 9.;
        double MATERIAL_QUANTIFIER = 1;
    };

    template<Piece_t piece>
    constexpr double pieceValue(EvaluationParams params) {
        if constexpr (piece == Piece::Pawn) {
            return params.MATERIAL_WEIGHT_PAWN;
        }
        if constexpr (piece == Piece::Knight) {
            return params.MATERIAL_WEIGHT_KNIGHT;
        }
        if constexpr (piece == Piece::Bishop) {
            return params.MATERIAL_WEIGHT_BISHOP;
        }
        if constexpr (piece == Piece::Rook) {
            return params.MATERIAL_WEIGHT_ROOK;
        }
        if constexpr (piece == Piece::Queen) {
            return params.MATERIAL_WEIGHT_QUEEN;
        }
        return 0;
    }

}

#endif //DORY_ENGINE_PARAMS_H
