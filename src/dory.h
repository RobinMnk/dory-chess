//
// Created by robin on 19.07.2024.
//

#ifndef DORY_DORY_H
#define DORY_DORY_H

#include "engine/search.h"
#include "utils/perft.h"
#include "utils/fenreader.h"

namespace Dory {

    class Engine {
        Search::Searcher searcher{};

    public:
        Engine() {
            PieceSteps::load();
            Zobrist::init(23984729);
        }

        void setHashTableSize(size_t tableSize) {
            searcher.setTableSize(tableSize);
        }

        template<bool whiteToMove>
        Result searchDepth(Board& board, int depth) {
            return searcher.iterativeDeepening<whiteToMove, Search::Depth>(board, depth);
        }

        Result searchDepth(Board& board, int depth, bool whiteToMove) {
            if(whiteToMove) return searchDepth<true>(board, depth);
            return searchDepth<false>(board, depth);
        }

        template<bool whiteToMove>
        Result searchTime(Board& board, long millis) {
            return searcher.iterativeDeepening<whiteToMove, Search::Time>(board, millis);
        }

        Result searchTime(Board& board, long millis, bool whiteToMove) {
            if(whiteToMove) return searchTime<true>(board, millis);
            return searchTime<false>(board, millis);
        }

        template<bool whiteToMove>
        Result analyze(Board& board, int millis, int inc) {
            return searcher.analyze<whiteToMove>(board, millis, inc);
        }

        Result analyze(Board& board, bool whiteToMove, int millis, int inc) {
            if(whiteToMove) return analyze<true>(board, millis, inc);
            return analyze<false>(board, millis, inc);
        }

        [[nodiscard]] uint64_t nodesSearched() const { return searcher.nodesSearched; }

        [[nodiscard]] uint64_t tableLookups() const { return searcher.tableLookups; }
    };
}

namespace DoryUtils {

    void initialize() {
        Dory::PieceSteps::load();
        Dory::Zobrist::init(23984729);
    }

    template<bool whiteToMove>
    int staticEvaluation(Dory::Board& board) {
        return Dory::evaluation::evaluatePosition<whiteToMove>(board);
    }

    int staticEvaluation(Dory::Board& board, bool whiteToMove) {
        if(whiteToMove) return staticEvaluation<true>(board);
        return -staticEvaluation<false>(board);
    }

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
