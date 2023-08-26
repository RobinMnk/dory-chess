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

}

#endif //DORY_ENGINE_PARAMS_H
