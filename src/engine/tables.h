//
// Created by Robin on 15.03.2024.
//

#ifndef DORY_TABLES_H
#define DORY_TABLES_H

#include <unordered_map>
#include <iostream>
#include "../core/board.h"

namespace Dory {

    class TranspositionTable {
    public:
        struct TTEntry {
            int value;
            Move move;
            int8_t depthDiff;
            uint8_t flag;
        };
    private:
        constexpr static const TTEntry NullEntry{0, NULLMOVE, 0, 0};
        std::unordered_map<uint64_t, TTEntry> lookup_table;
    public:
        static const uint8_t TTFlagExact = 0, TTFlagLowerBound = 1, TTFlagUpperBound = 2;
//        unsigned long long lookups{0};

        void insert(uint64_t boardHash, int eval, Move move, int depthDiff, int alpha, int beta) {
            uint8_t flag;
            if (eval <= alpha)
                flag = TTFlagUpperBound;
            else if (eval >= beta)
                flag = TTFlagLowerBound;
            else flag = TTFlagExact;

            TTEntry entry{eval, move, static_cast<int8_t>(depthDiff), flag};
            lookup_table.insert_or_assign(boardHash, entry);
        }

        std::pair<TTEntry, bool> lookup(uint64_t boardHash, int &alpha, int &beta, int depthDiff) {
            auto res = lookup_table.find(boardHash);

            if (res != lookup_table.end()) {
                TTEntry entry = res->second;
                bool resultValid = false;
                if (entry.depthDiff >= depthDiff) {
                    if (entry.flag == TTFlagExact) {
                        resultValid = true;
                    } else if (entry.flag == TTFlagLowerBound) {
                        if (entry.value > alpha) alpha = entry.value;
                    } else if (entry.flag == TTFlagUpperBound) {
                        if (entry.value < beta) beta = entry.value;
                    }

                    if (alpha >= beta) {
                        resultValid = true;
                    }
                }
                // entry is returned whenever one is found, so we can use the stored move in next searches
                // even if the value is invalid
                return {entry, resultValid};
            }

            // Board position not stored in table
            return {NullEntry, false};
        }

        void reset() {
//            lookups = 0;
            lookup_table.clear();
//        lookup_table.reserve(145000);
        }

        size_t size() const { // in kB
            return lookup_table.size() * sizeof(TTEntry) / 1024;
        }
    };

    class RepetitionTable {
        std::vector<uint64_t> stack;

    public:
        void reset() {
            stack.clear();
        }

        void push(uint64_t boardHash) {
            stack.push_back(boardHash);
        }

        void pop() {
            if (!stack.empty())
                stack.pop_back();
        }

        /// Check for threefold repetition.
        [[nodiscard]] bool check(uint64_t boardHash) const {
            int count = 0;
            for (uint64_t h : stack) {
                if (h == boardHash) {
                    count++;
                    if (count >= 2)
                        return true; // second occurrence = repetition
                }
            }
            return false;
        }
    };

} // namespace Dory

#endif //DORY_TABLES_H
