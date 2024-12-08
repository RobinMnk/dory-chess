//
// Created by Robin on 24.08.2023.
//

#ifndef DORY_EVALUATION_H
#define DORY_EVALUATION_H

#include <iostream>

#include "../core/checklogichandler.h"
#include "features.h"
#include "engineparams.h"

namespace Dory::evaluation {

    /**
     * Gives estimate for position evaluation score for the side that is to move.
     * Positive value is good for the side to move (not necessarily good for white)
     */
    template<bool whiteToMove>
    int evaluatePosition(const Board& board) {
        int matFriendly = features::material<whiteToMove>(board);
        int matEnemy = features::material<!whiteToMove>(board);

        // includes material!
        int activity = features::activity<whiteToMove>(board) - features::activity<!whiteToMove>(board);

        int passedPawns = features::passedPawns<whiteToMove>(board, matEnemy) - features::passedPawns<!whiteToMove>(board, matFriendly);

        int evalEstimate = activity + passedPawns;

        return evalEstimate;
    }

} // namespace Dory::evaluation


#endif //DORY_EVALUATION_H
