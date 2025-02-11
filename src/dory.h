//
// Created by robin on 19.07.2024.
//

#ifndef DORY_DORY_H
#define DORY_DORY_H

#include "engine/search.h"
#include "utils/fenreader.h"

namespace Dory {

    class Dory {
        Search::Searcher searcher{};

    public:
        Dory() {
            PieceSteps::load();
            Zobrist::init(23984729);
        }

        template<bool whiteToMove>
        Result searchDepth(const Board& board, int depth) {
            return searcher.iterativeDeepening<whiteToMove>(board, depth);
        }

        Result searchDepth(const Board& board, int depth, bool whiteToMove) {
            if(whiteToMove) return searchDepth<true>(board, depth);
            return searchDepth<false>(board, depth);
        }

        [[nodiscard]] uint64_t nodesSearched() const { return searcher.nodesSearched; }

        [[nodiscard]] uint64_t tableLookups() const { return searcher.tableLookups; }

        [[nodiscard]] size_t trTableSizeKb() const { return searcher.trTableSizeKb(); }

        [[nodiscard]] size_t trTableSizeMb() const { return searcher.trTableSizeMb(); }
    };

    void initialize() {
        PieceSteps::load();
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

    std::string parseEval(int eval) {
        return Dory::Utils::parseEval(eval);
    }

    void printLine(const std::vector<Dory::Move>& line, int eval) {
        Dory::Utils::printLine(line, eval);
    }

}



#endif //DORY_DORY_H
