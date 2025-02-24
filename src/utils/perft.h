//
// Created by robin on 19.07.2024.
//

#ifndef DORY_PERFT_H
#define DORY_PERFT_H

#include "../../src/core/movecollectors.h"

namespace DoryUtils {

    template<bool whiteToMove, int depth>
    std::vector<unsigned long long> perft(Dory::Board& board) {
        using namespace Dory::MoveCollectors;
        nodes.clear();
        nodes.resize(depth + 1);
        PerftCollector<depth>::template generateGameTree<whiteToMove>(board);
        return nodes;
    }

    template<int depth>
    std::vector<unsigned long long> perft(Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) return perft<true, depth>(board);
        else return perft<false, depth>(board);
    }

    std::vector<unsigned long long> perft(Dory::Board& board, bool whiteToMove, int depth) {
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
            case 10: return perft<10>(board, whiteToMove);
            case 11: return perft<11>(board, whiteToMove);
            default: throw std::runtime_error("Perft Depth not implemented!");
        }
    }

     // - - - - - - - - PERFT - - - - - - - -

    template<bool whiteToMove, int depth>
    unsigned long long perftSingleDepth(Dory::Board& board) {
        using namespace Dory::MoveCollectors;
        LimitedDFS<1>::totalNodes = 0;
        LimitedDFS<depth>::template generateGameTree<whiteToMove>(board);
        return LimitedDFS<1>::totalNodes;
    }

    template<int depth>
    unsigned long long perftSingleDepth(Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) return perftSingleDepth<true, depth>(board);
        else return perftSingleDepth<false, depth>(board);
    }

    unsigned long long perftSingleDepth(Dory::Board& board, bool whiteToMove, int depth) {
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
            case 10: return perftSingleDepth<10>(board, whiteToMove);
            case 11: return perftSingleDepth<11>(board, whiteToMove);
            default: throw std::runtime_error("Perft Depth not implemented!");
        }
    }

    // - - - - - - - - DIVIDE - - - - - - - -

    template<bool whiteToMove, int depth>
    void printDivide(Dory::Board& board) {
        using namespace Dory::MoveCollectors;
        Divide<depth>::template generateGameTree<whiteToMove>(board);
        Divide<depth>::print();
    }

    template<int depth>
    void printDivide(Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) printDivide<true, depth>(board);
        else printDivide<false, depth>(board);
    }

    void printDivide(Dory::Board& board, bool whiteToMove, int depth) {
        switch (depth) {
            case 1: printDivide<1>(board, whiteToMove); break;
            case 2: printDivide<2>(board, whiteToMove); break;
            case 3: printDivide<3>(board, whiteToMove); break;
            case 4: printDivide<4>(board, whiteToMove); break;
            case 5: printDivide<5>(board, whiteToMove); break;
            case 6: printDivide<6>(board, whiteToMove); break;
            case 7: printDivide<7>(board, whiteToMove); break;
            case 8: printDivide<8>(board, whiteToMove); break;
            case 9: printDivide<9>(board, whiteToMove); break;
            case 10: printDivide<10>(board, whiteToMove); break;
            case 11: printDivide<11>(board, whiteToMove); break;
            default: throw std::runtime_error("Divide Depth not implemented!");
        }
    }

} // namespace DoryUtils

#endif //DORY_PERFT_H
