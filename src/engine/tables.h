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
            uint32_t keyTag;
            uint8_t flag{TTFlagEmpty};
            uint8_t age;
            uint16_t depthSearched;
        };

        struct alignas(64) TTCluster {
            TTEntry entries[4];
        };

    private:
        static constexpr TTEntry NullEntry{0, NULLMOVE, 0, 0, 0, 0};
        std::vector<TTCluster> lookup_table;
        uint64_t mask;
        uint8_t generation{0};

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

        std::pair<TTEntry, bool> unpackEntry(TTEntry& entry, int &alpha, int &beta, int depthNeeded, int distanceFromRoot) {
            // entry not searched deep enough
            if (entry.depthSearched < depthNeeded)
                return {entry, false};

            int decoded = decodeMateScore(entry.value, distanceFromRoot);

            bool resultValid = false;
            switch (entry.flag) {
                case TTFlagExact:
                    resultValid = true;
                    break;

                case TTFlagLowerBound:
                    if (decoded > alpha) alpha = decoded;
                    if (alpha >= beta) resultValid = true;
                    break;

                case TTFlagUpperBound:
                    if (decoded < beta) beta = decoded;
                    if (alpha >= beta) resultValid = true;
                    break;

                default:
                    break;
            }

            TTEntry result = entry;
            result.value = decoded;
            return {result, resultValid};
        }

        inline int score(const TTEntry& e) const {
            // lower = more likely to be replaced
            return ((e.flag != TTFlagEmpty) << 17)
                   | ((e.age == generation) << 16)
                   | (0xFFFF - e.depthSearched);
        }

        static inline uint32_t extractKeyTag(uint64_t hash) {
            return static_cast<uint32_t>(hash >> 32);
        }

    public:
        static const uint8_t TTFlagEmpty = 0, TTFlagExact = 1, TTFlagLowerBound = 2, TTFlagUpperBound = 3;

        TranspositionTable(size_t sizeMB=128) : lookup_table(1024 * 16 * sizeMB), mask{1024 * 16 * sizeMB - 1} {}

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

            TTEntry entry{storedEval, move, extractKeyTag(boardHash), flag, generation, static_cast<uint16_t>(depthSearched)};
            size_t index = boardHash & mask;

            TTCluster& cluster = lookup_table[index];
            int bestIx = 0;
            int bestScore = score(cluster.entries[0]);

            for (int i = 1; i < 4; ++i) {
                int s = score(cluster.entries[i]);
                if (s < bestScore) {
                    bestIx = i;
                    bestScore = s;
                }
            }
            cluster.entries[bestIx] = entry;
        }

        // Lookup returns the TT entry and whether it can be used for evaluation
        std::pair<TTEntry, bool> lookup(uint64_t boardHash, int &alpha, int &beta, int depthNeeded, int distanceFromRoot) {
            size_t index = boardHash & mask;
            TTCluster& cluster = lookup_table[index];

            for(int i = 0; i < 4; i++) {
                TTEntry& entry = cluster.entries[i];
                if(entry.age == generation && entry.flag != TTFlagEmpty && extractKeyTag(boardHash) == entry.keyTag)
                    return unpackEntry(entry, alpha, beta, depthNeeded, distanceFromRoot);
            }
            return {NullEntry, false};
        }

        void reset() {
            generation++;
        }

        size_t size() const {
            return (lookup_table.size() * sizeof(TTCluster) / 1024);
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
