//
// Created by robin on 24.08.2023.
//

#ifndef DORY_EVALUATION_H
#define DORY_EVALUATION_H

#include "features.h"
#include "engine_params.h"

namespace evaluation {

    double hard_evaluate(const Board& board) {
        engine_params::EvaluationParams params;
        return features::material(board, params);
    }



};


#endif //DORY_EVALUATION_H
