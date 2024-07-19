//
// Created by Robin on 24.08.2023.
//

#ifndef DORY_EVALUATION_H
#define DORY_EVALUATION_H

#include <iostream>

#include "../core/checklogichandler.h"
#include "features.h"
#include "engineparams.h"

namespace Dory::Evaluation {

    /**
     * Gives estimate for position Evaluation score for the side that is to move.
     * Positive value is good for the side to move (not necessarily good for white)
     */
    template<bool whiteToMove>
    int evaluatePosition(const Board& board) {
        int matFriendly = features::material<whiteToMove>(board);
        int matEnemy = features::material<!whiteToMove>(board);
        int material = matFriendly - matEnemy;
//
//        int mobility = features::mobility<true>(board) - features::mobility<false>(board);
//
        int activity = features::activity<whiteToMove>(board) - features::activity<!whiteToMove>(board);

//        int evalEstimate = material * params.MATERIAL_QUANTIFIER
////                + mobility * params.MOBILITY_QUANTIFIER
//                + activity * params.ACTIVITY_QUANTIFIER;
//
//        evalEstimate /= 2; // seems important, not sure why -> because of the aspiration window!

        int passedPawns = features::passedPawns<whiteToMove>(board, matEnemy) - features::passedPawns<!whiteToMove>(board, matFriendly);

        int evalEstimate = material + activity + passedPawns;

        return evalEstimate / 16;
    }

} // namespace Dory::Evaluation


#endif //DORY_EVALUATION_H
