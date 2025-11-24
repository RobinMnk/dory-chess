//
// Created by Robin on 15.03.2024.
//

#ifndef DORY_TABLES_H
#define DORY_TABLES_H

#include <unordered_map>
#include <iostream>
#include "../core/board.h"

namespace Dory::Search {

    const int MATE_THRESHOLD = INF - 2000;

    class TranspositionTable {
    public:
        struct TTEntry {
            int value;
            Move move;
            int depthSearched;
            uint8_t flag;
        };

    private:
        constexpr static const TTEntry NullEntry{0, NULLMOVE, 0, 0};
        std::unordered_map<uint64_t, TTEntry> lookup_table;

        static inline int decodeMateScore(int score, int depth) {
            if (score > MATE_THRESHOLD) {
                return score - depth;
            } else if (score < -MATE_THRESHOLD) {
                return score + depth;
            }
            return score;
        }

        static inline int encodeMateScore(int score, int depth) {
            if (score > MATE_THRESHOLD) {
                return INF - depth;
            } else if (score < -MATE_THRESHOLD) {
                return -INF + depth;
            }
            return score;
        }

    public:
        static const uint8_t TTFlagExact = 0, TTFlagLowerBound = 1, TTFlagUpperBound = 2;

        void insert(uint64_t boardHash, int eval, Move move, int depthSearched, int alpha, int beta, int distanceFromRoot) {
            uint8_t flag;
            if (eval <= alpha)
                flag = TTFlagUpperBound;
            else if (eval >= beta)
                flag = TTFlagLowerBound;
            else
                flag = TTFlagExact;

            // Encode mate score relative to root
            int storedEval = encodeMateScore(eval, distanceFromRoot);

            TTEntry entry{storedEval, move, depthSearched, flag};
            lookup_table.insert_or_assign(boardHash, entry);
        }

        // Lookup returns the TT entry and whether it can be used for evaluation
        std::pair<TTEntry, bool> lookup(uint64_t boardHash, int &alpha, int &beta, int depthNeeded, int distanceFromRoot) {
            auto it = lookup_table.find(boardHash);
            if (it == lookup_table.end())
                return {NullEntry, false};

            TTEntry entry = it->second;

            // entry not searched deep enough
            if (entry.depthSearched < depthNeeded)
                return {entry, false};

            entry.value = decodeMateScore(entry.value, distanceFromRoot);

            bool resultValid = false;
            switch (entry.flag) {
                case TTFlagExact:
                    resultValid = true;
                    break;

                case TTFlagLowerBound:
                    if (entry.value > alpha) alpha = entry.value;
                    if (alpha >= beta) resultValid = true;
                    break;

                case TTFlagUpperBound:
                    if (entry.value < beta) beta = entry.value;
                    if (alpha >= beta) resultValid = true;
                    break;

                default:
                    break;
            }

            // Return entry for move ordering regardless of resultValid
            return {entry, resultValid};
        }

        void reset() {
            lookup_table.clear();
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
