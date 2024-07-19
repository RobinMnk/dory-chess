//
// Created by robin on 19.07.2024.
//

#ifndef DORY_PERFT_H
#define DORY_PERFT_H

#include "../../src/core/movecollectors.h"

namespace DoryUtils {

    template<bool whiteToMove, int depth>
    std::vector<unsigned long long> perft(const Dory::Board& board) {
        using namespace Dory::MoveCollectors;
        nodes.clear();
        nodes.resize(depth + 1);
        PerftCollector<depth>::template generateGameTree<whiteToMove>(board);
        return nodes;
    }

    template<int depth>
    std::vector<unsigned long long> perft(const Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) return perft<true, depth>(board);
        else return perft<false, depth>(board);
    }


    template<bool whiteToMove, int depth>
    unsigned long long perftSingleDepth(const Dory::Board& board) {
        using namespace Dory::MoveCollectors;
        LimitedDFS<1>::totalNodes = 0;
        LimitedDFS<depth>::template generateGameTree<whiteToMove>(board);
        return LimitedDFS<1>::totalNodes;
    }

    template<int depth>
    unsigned long long perftSingleDepth(const Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) return perftSingleDepth<true, depth>(board);
        else return perftSingleDepth<false, depth>(board);
    }

}

#endif //DORY_PERFT_H
