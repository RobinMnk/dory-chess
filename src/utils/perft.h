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

    std::vector<unsigned long long> perft(const Dory::Board& board, bool whiteToMove, int depth) {
        switch (depth) {
            case 1: return perft<1>(board, whiteToMove);
            case 2: return perft<2>(board, whiteToMove);
            case 3: return perft<3>(board, whiteToMove);
            case 4: return perft<4>(board, whiteToMove);
            case 5: return perft<5>(board, whiteToMove);
            case 6: return perft<6>(board, whiteToMove);
            case 7: return perft<7>(board, whiteToMove);
            case 8: return perft<8>(board, whiteToMove);
            case 9: return perft<9>(board, whiteToMove);
            default: throw std::runtime_error("Perft Depth not implemented!");
        }
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

    unsigned long long perftSingleDepth(const Dory::Board& board, bool whiteToMove, int depth) {
        switch (depth) {
            case 1: return perftSingleDepth<1>(board, whiteToMove);
            case 2: return perftSingleDepth<2>(board, whiteToMove);
            case 3: return perftSingleDepth<3>(board, whiteToMove);
            case 4: return perftSingleDepth<4>(board, whiteToMove);
            case 5: return perftSingleDepth<5>(board, whiteToMove);
            case 6: return perftSingleDepth<6>(board, whiteToMove);
            case 7: return perftSingleDepth<7>(board, whiteToMove);
            case 8: return perftSingleDepth<8>(board, whiteToMove);
            case 9: return perftSingleDepth<9>(board, whiteToMove);
            default: throw std::runtime_error("Perft Depth not implemented!");
        }
    }

} // namespace DoryUtils

#endif //DORY_PERFT_H
