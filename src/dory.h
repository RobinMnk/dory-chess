//
// Created by robin on 19.07.2024.
//

#ifndef DORY_DORY_H
#define DORY_DORY_H

#include "engine/search.h"
#include "utils/fenreader.h"

namespace Dory {

    bool readyOk = false;

    void initialize() {
        if(readyOk) return;
        Dory::PieceSteps::load();
        Dory::Zobrist::init(23984729);
        readyOk = true;
    }

    template<bool whiteToMove>
    Result searchDepth(const Board& board, int depth) {
        return Search::Searcher::iterativeDeepening<whiteToMove>(board, depth);
    }

    Result searchDepth(const Board& board, int depth, bool whiteToMove) {
        if(whiteToMove) return searchDepth<true>(board, depth);
        return searchDepth<false>(board, depth);
    }

    template<bool whiteToMove>
    int staticEvaluation(const Board& board) {
        return evaluation::evaluatePosition<whiteToMove>(board);
    }

    int staticEvaluation(const Board& board, bool whiteToMove) {
        if(whiteToMove) return staticEvaluation<true>(board);
        return -staticEvaluation<false>(board);
    }
}

namespace DoryUtils {

    std::pair<Dory::Board, bool> parseFEN(const std::string_view& fen) {
        return Dory::Utils::parseFEN(fen);
    }

    void printLine(const std::vector<Dory::Move>& line, int eval) {
        Dory::Utils::printLine(line, eval);
    }

}



#endif //DORY_DORY_H
