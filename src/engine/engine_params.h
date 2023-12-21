//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_PARAMS_H
#define DORY_ENGINE_PARAMS_H

namespace engine_params {

    struct EvaluationParams {
        const int  MATERIAL_WEIGHT_PAWN = 100;
        const int  MATERIAL_WEIGHT_KNIGHT = 300;
        const int  MATERIAL_WEIGHT_BISHOP = 325;
        const int  MATERIAL_WEIGHT_ROOK = 500;
        const int  MATERIAL_WEIGHT_QUEEN = 900;
        const int  MATERIAL_QUANTIFIER = 1;

        const int  MOBILITY_WEIGHT_PAWN = 1;
        const int  MOBILITY_WEIGHT_KNIGHT = 3;
        const int  MOBILITY_WEIGHT_BISHOP = 4;
        const int  MOBILITY_WEIGHT_ROOK = 4;
        const int  MOBILITY_WEIGHT_QUEEN = 7;
        const int  MOBILITY_WEIGHT_KING = -10;
        const int  MOBILITY_QUANTIFIER = 1;
    };

    template<Piece_t piece>
    constexpr int pieceValue(EvaluationParams params) {
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
