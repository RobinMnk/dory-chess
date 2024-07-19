//
// Created by robin on 19.07.2024.
//

#ifndef DORY_PERFT_H
#define DORY_PERFT_H

#include "../../src/core/movecollectors.h"

namespace DoryUtils {

    template<bool whiteToMove, int depth>
    void perft(const Dory::Board& board) {
        Dory::MoveCollectors::nodes.clear();
        Dory::MoveCollectors::nodes.resize(depth + 1);
        Dory::MoveCollectors::PerftCollector<depth>::template generateGameTree<whiteToMove>(board);
    }

    template<int depth>
    void perft(const Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) perft<true, depth>(board);
        else perft<false, depth>(board);
    }


    template<bool whiteToMove, int depth>
    void perftSingleDepth(const Dory::Board& board) {
        Dory::MoveCollectors::LimitedDFS<1>::totalNodes = 0;
        Dory::MoveCollectors::LimitedDFS<depth>::template generateGameTree<whiteToMove>(board);
    }

    template<int depth>
    void perftSingleDepth(const Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) perftSingleDepth<true, depth>(board);
        else perftSingleDepth<false, depth>(board);
    }


}

#endif //DORY_PERFT_H
