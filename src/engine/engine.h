//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_H
#define DORY_ENGINE_H

#include <iostream>
#include <stack>
#include <vector>
#include <memory>
#include <thread>
#include <unordered_map>
#include "evaluation.h"
#include "../utils.h"
#include "../zobrist.h"
#include "../movegen.h"

const float INF = 99999;
unsigned int NUM_LINES = 1;
const float BEST_MOVE_MARGIN = 0.1;
const int MAX_ITER_DEPTH = 6;

using Line = std::vector<Move>;
using NMR = std::pair<float, Line>;


float subjectiveEval(float eval, State state) {
    return state.whiteToMove ? eval : -eval;
}

bool sortMovePairs(const std::pair<float, Move> &a, const std::pair<float, Move> &b) {
    return a.first > b.first;
}


class TranspositionTable {
    static const uint8_t TTFlagExact = 0, TTFlagLowerBound = 1, TTFlagUpperBound = 2;

    struct TTEntry {
        float value;
        uint8_t depth, flag;
    };
    std::unordered_map<BB, TTEntry> lookup_table;

public:
    unsigned long long lookups{0};
    bool resultValid{false};

    void insert(float eval, size_t boardHash, float alpha, float beta, int depth) {
        uint8_t flag;
        if (eval <= alpha)
            flag = TTFlagUpperBound;
        else if (eval >= beta)
            flag = TTFlagLowerBound;
        else flag = TTFlagExact;

        TTEntry entry{ eval, static_cast<uint8_t>(depth), flag};
        lookup_table.emplace(boardHash, entry);
    }

    NMR lookup(size_t boardHash, float& alpha, float& beta, int depth) {
        auto res = lookup_table.find(boardHash);

        resultValid = false;
        if(res != lookup_table.end()) {
            lookups++;
            TTEntry entry = res->second;
            if (entry.depth >= depth) {
                if (entry.flag == TTFlagExact) {
                    resultValid = true;
                    return { entry.value, {} };
                } else if (entry.flag == TTFlagLowerBound) {
                    if (entry.value > alpha) alpha = entry.value;
                } else if (entry.flag == TTFlagUpperBound) {
                    if (entry.value < beta) beta = entry.value;
                }

                resultValid = true;
                if (alpha >= beta) return { entry.value, {} };
            }
        }
        return { 0, {} };
    }

    void reset() {
        lookup_table.clear();
        lookups = 0;
    }

    size_t size() {
        return lookup_table.size() * sizeof(TTEntry) / 1024;
    }
};

class EngineMC {
public:
    static TranspositionTable trTable;
    static float evaluation;
    static std::vector<std::pair<Line, float>> bestLines;
    static BB nodesSearched;

    static void iterativeDeepening(const Board& board, const State state) {
        reset();
        for(int depth = 1; depth < MAX_ITER_DEPTH; depth++) {
            searchDepth(board, state, depth);
        }
    }

    static NMR searchDepth(const Board& board, const State state, int md) {
        maxDepth = md;
        bestLines.clear();
        auto [ev, line] = negamax<true, false>(board, state, 1, -INF, INF);
        return {subjectiveEval(ev, state), line};
    }


    static Line bestMoves() {
        Line res;
        for(auto& [line, eval]: bestLines) {
            res.push_back(line.back());
        }
        return res;
    }

    static std::vector<std::pair<float, Move>> topLevelLegalMoves() {
        return moves[1];
    }

    static void reset() {
        trTable.reset();
        nodesSearched = 0;
    }

private:
    static std::array<std::vector<std::pair<float, Move>>, 48> moves;
    static int maxDepth, currentDepth;

    template<bool topLevel, bool quiescene>
    static NMR negamax(const Board& board, const State state, int depth, float alpha, float beta) {
        /// Lookup position in table
        size_t boardHash{0};
        float origAlpha = alpha;
        if constexpr (!topLevel) {
            boardHash = Zobrist::hash(board, state);
            NMR res = trTable.lookup(boardHash, alpha, beta, depth);
            if (trTable.resultValid) {
                return res;
            }
        }

        /// Recursion Base Case: Max Depth reached -> return heuristic position eval

        if constexpr (quiescene) {
            float stand_pat = subjectiveEval(evaluation::position_evaluate(board), state);

            if (stand_pat >= beta) {
                nodesSearched++;
                return { beta, {} };
            }
            if (alpha < stand_pat) {
                alpha = stand_pat;
            }
        } else {
            if (depth > maxDepth) {
                return negamax<false, true>(board, state, depth, alpha, beta);
            }
        }

        moves.at(depth).clear();
        currentDepth = depth;

        generate<EngineMC, quiescene>(board, state);

        if constexpr (quiescene) {
            if (moves.at(depth).empty()) {
                nodesSearched++;
                return {alpha, {}};
            }
        } else {
            bool checkmated = MoveGenerator<EngineMC>::pd->inCheck() && moves.at(depth).empty();
            if (checkmated) {
                nodesSearched++;
                float eval = -(INF - static_cast<float>(depth));
                trTable.insert(eval, boardHash, origAlpha, beta, depth);
                return {eval, {}};
            }

            if (moves.at(depth).empty()) {
                // Stalemate!
                nodesSearched++;
                trTable.insert(0, boardHash, origAlpha, beta, depth);
                return {0, {}};
            }
        }

//        }

        std::sort(moves.at(depth).begin(), moves.at(depth).end(), sortMovePairs);

//        if constexpr (topLevel) {
//            for(auto& x: moves.at(depth)) {
//                std::cout << "(" << x.first << ", " << Utils::moveNameNotation(x.second) << ") ";
//            }
//            std::cout << std::endl;
//        }


        float currentEval = -1000000;
        Line bestLine;

        /// Iterate through all moves
        for(auto& move_pair: moves.at(depth)) {
            auto [info, move] = move_pair;

//            for (int i = 0; i < depth; i++)
//                std::cout << " ";
//            std::cout << depth << " : " << Utils::moveNameNotation(move) << std::endl;


//            if(expandMove) {
            auto [nextBoard, nextState] = forkBoard(board, state, move);
            auto [eval, line] = negamax<false, quiescene>(nextBoard, nextState, depth + 1,  -beta,  -alpha);
            eval = -eval;

//            if constexpr (topLevel) {
//                std::cout << "EVAL: " << subjectiveEval(eval, state) << std::endl;
//                Utils::printMove(move);
//            }

            if constexpr (quiescene) {
                if (eval >= beta) {
                    return { beta, {} };
                }
                if (eval > alpha) {
                    alpha = eval;
                    line.push_back(move);
                    bestLine = line;
                }
            } else {
                if (eval > currentEval) {
                    currentEval = eval;
                    line.push_back(move);
                    bestLine = line;

                    if constexpr (topLevel) {
//                        Utils::printLine(line, subjectiveEval(eval, state));

                        bestLines.clear();
                        bestLines.emplace_back(line, subjectiveEval(eval, state));
                    }
                } else {
                    if constexpr (topLevel) {
                        if (bestLines.size() < NUM_LINES) {
                            if (eval >= currentEval - BEST_MOVE_MARGIN) {
                                line.push_back(move);
                                bestLines.emplace_back(line, subjectiveEval(eval, state));
                            }
                        }
                    }
                }

                if constexpr (!topLevel) {
                    if (currentEval > alpha) {
                        alpha = currentEval;
                    }
                    if (alpha >= beta) break;
                }
            }
        }

        /// Save to lookup table
        if constexpr (!topLevel) {
            trTable.insert(currentEval, boardHash, origAlpha, beta, depth);
        }

        if constexpr (quiescene) {
            return { alpha, bestLine };
        }
        return {currentEval, bestLine};
    }

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {
        moves[currentDepth].emplace_back(
            evaluation::move_heuristic<state, piece, flags>(board, from, to),
            Move(from, to, piece, flags)
        );
    }

    friend class MoveGenerator<EngineMC, true>;
    friend class MoveGenerator<EngineMC, false>;
};

std::array<std::vector<std::pair<float, Move>>, 48> EngineMC::moves{};
int EngineMC::currentDepth{0};
int EngineMC::maxDepth{0};
BB EngineMC::nodesSearched{0};
std::vector<std::pair<Line, float>> EngineMC::bestLines{};
TranspositionTable EngineMC::trTable{};

#endif //DORY_ENGINE_H
