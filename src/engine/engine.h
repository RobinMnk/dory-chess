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

const int INF = 999;
unsigned int NUM_LINES = 1;
const int BEST_MOVE_MARGIN = 10;
const int MAX_ITER_DEPTH = 5;

using Line = std::vector<Move>;
using NMR = std::pair<int, Line>;


int subjectiveEval(int eval, State state) {
    return state.whiteToMove ? eval : -eval;
}

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
        lookup_table.reserve(145000);
    }

    size_t size() {
        return lookup_table.size() * sizeof(TTEntry) / 1024;
    }
};

class EngineMC {
    static Move priorityMove;
public:
    static TranspositionTable trTable;
    static int evaluation;
    static std::vector<std::pair<Line, int>> bestLines;
    static BB nodesSearched;
    static Move bestMove;

    static NMR iterativeDeepening(const Board& board, const State state, int md=MAX_ITER_DEPTH) {
        reset();
        Line bestLine;
        int bestEval = -100000;
        for(int depth = 1; depth <= md; depth++) {
            std::cout << "Searching Depth " << depth << std::endl;
            auto [eval, line] = searchDepth(board, state, depth);
            eval = subjectiveEval(eval, state);
            if (eval > bestEval) {
                bestEval = eval;
                bestLine = line;
                Utils::printLine(line, eval);
            }
        }
        return { bestEval, bestLine };
    }

    static NMR searchDepth(const Board& board, const State state, int md) {
        maxDepth = md;
        bestLines.clear();
        auto [ev, line] = negamax<true, false>(board, state, 1, -INF, INF);
        return {subjectiveEval(ev, state), line};
    }

    static bool sortMovePairs(const std::pair<float, Move> &a, const std::pair<float, Move> &b) {
        return a.first > b.first;
    }

    static Line bestMoves() {
        Line res;
        for(auto& [line, eval]: bestLines) {
            res.push_back(line.back());
        }
        return res;
    }

    static std::vector<std::pair<int, Move>> topLevelLegalMoves() {
        return moves[1];
    }

    static void reset() {
        trTable.reset();
        nodesSearched = 0;
    }

private:
    static std::array<std::vector<std::pair<int, Move>>, 128> moves;
    static int maxDepth, currentDepth;

    template<bool topLevel, bool quiescene>
    static NMR negamax(const Board& board, const State state, int depth, int alpha, int beta) {
        /// Lookup position in table
        int origAlpha = alpha;
        size_t boardHash = Zobrist::hash(board, state);
        auto [ttEntry, resultValid] = trTable.lookup(boardHash, alpha, beta, maxDepth - depth);
        if (resultValid) {
            trTable.lookups++;
            return { ttEntry.value, {} };
        }

        /// Recursion Base Case: Max Depth reached -> return heuristic position eval
        if constexpr (quiescene) {
            int stand_pat = subjectiveEval(evaluation::position_evaluate(board), state);

            if (stand_pat >= beta) {
                nodesSearched++;
                trTable.insert(boardHash, beta, NULLMOVE, maxDepth - depth, origAlpha, beta);
                return { beta, {} };
            }
            if (alpha < stand_pat) {
                alpha = stand_pat;
            }
        } else {
            if (depth > maxDepth) {
//                nodesSearched++;
//                int eval = subjectiveEval(evaluation::position_evaluate(board), state);
//                trTable.insert(boardHash, eval, NULLMOVE, maxDepth - depth, origAlpha, beta);
//                return { eval, {} };

                return negamax<false, true>(board, state, depth, alpha, beta);
            }
        }

        if constexpr (topLevel) {
            priorityMove = bestMove;
        } else {
            // ttEntry.move may be NULLMOVE, but that does not hurt us
            priorityMove = ttEntry.move;
        }

        moves.at(depth).clear();
        currentDepth = depth;

        generate<EngineMC, quiescene>(board, state);

        if constexpr (quiescene) {
            if (moves.at(depth).empty()) {
                nodesSearched++;
                trTable.insert(boardHash, alpha, NULLMOVE, maxDepth - depth, origAlpha, beta);
                return {alpha, {}};
            }
        } else {
            bool checkmated = MoveGenerator<EngineMC>::pd->inCheck() && moves.at(depth).empty();
            if (checkmated) {
                nodesSearched++;
                int eval = -(INF - static_cast<int>(depth));
                trTable.insert(boardHash, eval, NULLMOVE, maxDepth - depth, origAlpha, beta);
                return {eval, {}};
            }

            if (moves.at(depth).empty()) {
                // Stalemate!
                nodesSearched++;
                trTable.insert(boardHash, 0, NULLMOVE, maxDepth - depth, origAlpha, beta);
                return {0, {}};
            }
        }


        std::sort(moves.at(depth).begin(), moves.at(depth).end(), sortMovePairs);

        Line localBestLine;
        Move localBestMove;

        /// Iterate through all moves
        for(auto& move_pair: moves.at(depth)) {
            auto [info, move] = move_pair;

            auto [nextBoard, nextState] = forkBoard(board, state, move);
            auto [eval, line] = negamax<false, quiescene>(nextBoard, nextState, depth + 1,  -beta,  -alpha);
            eval = -eval;

//            for (int i = 0; i < depth; i++)
//                std::cout << " ";
//            std::cout << depth << " : " << Utils::moveNameNotation(move) << "  " << eval << std::endl;

//            if constexpr (topLevel) {
//                std::cout << "EVAL: " << subjectiveEval(eval, state) << std::endl;
//                Utils::printMove(move);
//            }

            if (eval > alpha) {
                alpha = eval;
                line.push_back(move);
                localBestLine = line;
                localBestMove = move;

                if constexpr (topLevel) {
                    bestLines.clear();
                    bestLines.emplace_back(line, subjectiveEval(eval, state));
                    bestMove = move;
                }
            } else {
                if constexpr (topLevel) {
                    if (bestLines.size() < NUM_LINES) {
                        if (eval >= alpha - BEST_MOVE_MARGIN) {
                            line.push_back(move);
                            bestLines.emplace_back(line, subjectiveEval(eval, state));
                        }
                    }
                }
            }

            if (alpha >= beta) {
                break;
            }
        }

        /// Save to lookup table
        trTable.insert(boardHash, alpha, localBestMove, maxDepth - depth, origAlpha, beta);

        return {alpha, localBestLine };
    }

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {
        moves[currentDepth].emplace_back(
            evaluation::move_heuristic<state, piece, flags>(board, from, to, MoveGenerator<EngineMC>::pd, priorityMove),
            createMoveFromBB(from, to, piece, flags)
        );
    }

    friend class MoveGenerator<EngineMC, true>;
    friend class MoveGenerator<EngineMC, false>;
};

std::array<std::vector<std::pair<int, Move>>, 128> EngineMC::moves{};
int EngineMC::currentDepth{0};
int EngineMC::maxDepth{0};
BB EngineMC::nodesSearched{0};
std::vector<std::pair<Line, int>> EngineMC::bestLines{};
TranspositionTable EngineMC::trTable{};
Move EngineMC::bestMove{};
Move EngineMC::priorityMove{};

#endif //DORY_ENGINE_H
