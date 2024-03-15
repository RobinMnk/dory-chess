//
// Created by Robin on 15.03.2024.
//

#ifndef DORY_TABLES_H
#define DORY_TABLES_H

#include <unordered_map>
#include <iostream>
#include "../board.h"

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
    std::unordered_map<BB, TTEntry> lookup_table;
public:
    static const uint8_t TTFlagExact = 0, TTFlagLowerBound = 1, TTFlagUpperBound = 2;
    unsigned long long lookups{0};

    void insert(size_t boardHash, int eval, Move move, int depthDiff, int alpha, int beta) {
        uint8_t flag;
        if (eval <= alpha)
            flag = TTFlagUpperBound;
        else if (eval >= beta)
            flag = TTFlagLowerBound;
        else flag = TTFlagExact;

        TTEntry entry{ eval, move, static_cast<int8_t>(depthDiff), flag};
        lookup_table.emplace(boardHash, entry);
    }

    std::pair<TTEntry, bool> lookup(size_t boardHash, int& alpha, int& beta, int depthDiff) {
        auto res = lookup_table.find(boardHash);

        bool resultValid = false;
        if(res != lookup_table.end()) {
            TTEntry entry = res->second;
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
            return { entry, resultValid };
        }

        // Board position not stored in table
        return { NullEntry, false };
    }

    void reset() {
        lookups = 0;
        lookup_table.clear();
//        lookup_table.reserve(145000);
    }

    size_t size() {
        return lookup_table.size() * sizeof(TTEntry) / 1024;
    }
};

class RepetitionTable {
    std::unordered_map<size_t, uint8_t> table{};

public:
    /**
     * Call after pawn move or after capture
     */
    void reset() {
        table.clear();
    }

    void insert(size_t boardHash) {
        auto it = table.find(boardHash);
        if (it != table.end()) {
            it->second++;
        } else {
            table.emplace(boardHash, 1);
        }
    }

    void remove(size_t boardHash) {
        auto it = table.find(boardHash);
        if (it != table.end()) {
            it->second--;
//            if (it->second == 0) {
//                table.erase(it);
//            }
        }
    }

    void print() {
        for(auto& it: table) {
            if(it.second) {
                std::cout << it.first << ":  " << it.second << std::endl;
            }
        }
    }

    bool check(size_t boardHash) {
        auto it = table.find(boardHash);
        return it != table.end() && it->second == 2;
    }
};

#endif //DORY_TABLES_H
