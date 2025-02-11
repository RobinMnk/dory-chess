//
// Created by robin on 28.07.2024.
//

#ifndef DORY_MC_H
#define DORY_MC_H

#include "../core/board.h"
#include "search.h"

namespace Dory::MonteCarlo {
//
//    template<bool whiteToMove>
//    int handle(Board &board, int depth) {
////        auto [eval, line] = Dory::Search::Searcher::iterativeDeepening<whiteToMove>(board, depth);
////
////        if (line.empty()) {
////            // Game over
////            if (eval > (INF - 50)) {
////                // White wins by Checkmate
//////                    std::cout << "Checkmate - White wins!  (" << (ply / 2) << " moves)" << std::endl;
////                return whiteToMove ? 3 : 1;
////            } else if (eval < -(INF - 50)) {
////                // Black wins by Checkmate
//////                    std::cout << "Checkmate - Black wins!  (" << (ply / 2) << " moves)" << std::endl;
////                return whiteToMove ? 1 : 3;
////            }
////                std::cout << "Draw - Threefold repetition!  (" << (ply / 2) << " moves)" << std::endl;
//            return 2;
//        }
//
//        Move nextMove = line.back();
//        if constexpr (whiteToMove) {
//            std::cout << Utils::moveNameShortNotation(nextMove) << "  ";
//        } else {
//            std::cout << Utils::moveNameShortNotation(nextMove) << std::endl;
//        }
//
//        Dory::Search::Searcher::repTable.insert(Zobrist::hash<whiteToMove>(board));
//
//        board.makeMove<whiteToMove>(nextMove);
//
//        return 0;
//    }
//
//    template<bool whiteToMove>
//    int simulateGame(const Board &board, int depth, int maxMoves = 100) {
//        Board B{board};
//        int res;
//        for(int i = 0; i < maxMoves; ++i) {
//            res = handle<whiteToMove>(B, depth);
//            if(res) return res-1;
//            res = handle<!whiteToMove>(B, depth);
//            if(res) return res-1;
//        }
//        return 1;
//    }


} // namespace Dory::MonteCarlo

#endif //DORY_MC_H
