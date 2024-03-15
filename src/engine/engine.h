//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_H
#define DORY_ENGINE_H

#include "evaluation.h"
#include "../utils/utils.h"
#include "../utils/zobrist.h"
#include "../core/movegen.h"
#include "tables.h"

unsigned int NUM_LINES = 1;
const int BEST_MOVE_MARGIN = 10;
const int MAX_ITER_DEPTH = 5;
const int MAX_WINDOW_INCREASES = 3;

using Line = std::vector<Move>;
using NMR = std::pair<int, Line>;


int subjectiveEval(int eval, State state) {
    return state.whiteToMove ? eval : -eval;
}

class EngineMC {
    static Move priorityMove;

    static std::array<std::vector<std::pair<int, Move>>, 128> moves;
    static int maxDepth, currentDepth;
public:
    static TranspositionTable trTable;
    static RepetitionTable repTable;
    static std::vector<std::pair<Line, int>> bestLines;
    static BB nodesSearched;
    static Move bestMove;
    static std::vector<Move> bestMoves;

    static NMR iterativeDeepening(const Board& board, const State state, int md=MAX_ITER_DEPTH) {
        reset();
//        repTable.reset();
        Line bestLine;
        int bestEval = -INF, window = 20, windowIncreases = 0;
        int alpha = -INF, beta = INF;

        for(int depth = 1; depth <= md;) {
//            std::cout << "Searching Depth " << depth << "    (" << alpha << " / " << beta << ")" << std::endl;
            auto [eval, line] = searchDepth(board, state, depth, alpha, beta);

            /// Aspiration Window
            if(eval <= alpha || eval >= beta) {
                if (windowIncreases++ > MAX_WINDOW_INCREASES) {
                    alpha = -INF;
                    beta = INF;
                } else {
                    window *= 4;
                    alpha = bestEval - window;
                    beta = bestEval + window;
                }
                continue; // Search again with same depth
            }
            window = 20;
            windowIncreases = 0;
            alpha = eval - window;
            beta = eval + window;

            bestEval = eval;
            bestLine = line;

//            std::cout << "Line for depth " << depth << std::endl;
            std::cout << "Depth " << depth << " -> ";
            Utils::printLine(bestLine, bestEval);

            depth++;
        }
        return { bestEval, bestLine };
    }

    static NMR searchDepth(const Board& board, const State state, int depth, int alpha=-INF, int beta=INF) {
        maxDepth = depth;
        bestLines.clear();
        auto [ev, line] = negamax<true>(board, state, 1, alpha, beta);
        return {subjectiveEval(ev, state), line};
    }

    static bool sortMovePairs(const std::pair<float, Move> &a, const std::pair<float, Move> &b) {
        return a.first > b.first;
    }

    static std::vector<std::pair<int, Move>> topLevelLegalMoves() {
        return moves[1];
    }

    static void reset() {
        trTable.reset();
        repTable.reset();
//        nodesSearched = 0;
    }

private:

    template<bool topLevel>
    static NMR negamax(const Board& board, const State state, int depth, int alpha, int beta) {

        size_t boardHash = Zobrist::hash(board, state);
        /// Check for Threefold-Repetition
        if(repTable.check(boardHash)) {
            return {0, {}};
        }

        /// Lookup position in table
        int origAlpha = alpha;
        auto [ttEntry, resultValid] = trTable.lookup(boardHash, alpha, beta, maxDepth - depth);
        if (resultValid) {
            trTable.lookups++;
            return { ttEntry.value, {} };
        }

        /// Switch to Quiescence Search
        if (depth > maxDepth) {
//            nodesSearched++;
//            int eval = evaluation::evaluatePosition(board, state);
//            trTable.insert(boardHash, eval, NULLMOVE, maxDepth - depth, origAlpha, beta);
//            return { eval, {} };

//                return negamax<false, true>(board, state, depth, alpha, beta);

            return quiescenceSearch(board, state, depth, alpha, beta);
        }


        // Setup variables before generating legal moves
        if constexpr (topLevel) {
            priorityMove = bestMove;
        } else {
            // ttEntry.move may be NULLMOVE, but that does not hurt us
            priorityMove = ttEntry.move;
        }
        moves.at(depth).clear();
        currentDepth = depth;

        /// Generate legal moves
        generate<EngineMC>(board, state);

        // No legal moves available
        if(moves.at(depth).empty()) {
            if (MoveGenerator<EngineMC>::pd->inCheck()) {
                // Checkmate!
                nodesSearched++;
                int eval = -(INF - depth);
                trTable.insert(boardHash, eval, NULLMOVE, maxDepth - depth, origAlpha, beta);
                return {eval, {}};
            } else {
                // Stalemate!
                nodesSearched++;
                trTable.insert(boardHash, 0, NULLMOVE, maxDepth - depth, origAlpha, beta);
                return {0, {}};
            }
        }

        std::sort(moves.at(depth).begin(), moves.at(depth).end(), sortMovePairs);

        Line localBestLine;
        Move localBestMove;
        int bestEval = -INF;

        /// Iterate through all moves
        for(auto& [info, move]: moves.at(depth)) {

            repTable.insert(boardHash);

            auto [nextBoard, nextState] = forkBoard(board, state, move);
            auto [eval, line] = negamax<false>(nextBoard, nextState, depth + 1,  -beta,  -alpha);
            eval = -eval;

            repTable.remove(boardHash);

//            for (int i = 0; i < depth; i++)
//                std::cout << " ";
//            std::cout << depth << " : " << Utils::moveNameNotation(move) << "  " << eval << std::endl;

//            if constexpr (topLevel) {
//                Line l = std::vector<Move>{move};
//                Utils::printLine(l, eval);
//            }

            if(eval > alpha)
                alpha = eval;

            if (eval > bestEval) {
                bestEval = eval;
                line.push_back(move);
                localBestLine = line;
                localBestMove = move;

                if constexpr (topLevel) {
//                    bestLines.clear();
//                    bestLines.emplace_back(line, eval);
                    bestMove = move;
//                    bestMoves.clear();
//                    bestMoves.push_back(move);
                }
//            } else {
//                if constexpr (topLevel) {
//                    if (bestLines.size() < NUM_LINES) {
//                        if (eval >= alpha - BEST_MOVE_MARGIN) {
//                            line.push_back(move);
//                            bestLines.emplace_back(line, eval);
//                            bestMoves.push_back(move);
//                        }
//                    }
//                }
            }

            if (alpha >= beta) {
                break;
            }
        }

        /// Save to lookup table
        trTable.insert(boardHash, bestEval, localBestMove, maxDepth - depth, origAlpha, beta);

        return { bestEval, localBestLine };
    }

    static NMR quiescenceSearch(const Board& board, State state, int depth, int alpha, int beta) {
        /// Recursion Base Case: Max Depth reached -> return heuristic position eval
        int standPat = evaluation::evaluatePosition(board, state);

        if (standPat >= beta) {
            nodesSearched++;
            return { beta, {} };
        }
        if (alpha < standPat) {
            alpha = standPat;
        }

        moves.at(depth).clear();
        currentDepth = depth;

        generate<EngineMC, true>(board, state);

        if (moves.at(depth).empty()) {
            nodesSearched++;
            return {alpha, {}};
        }

        std::sort(moves.at(depth).begin(), moves.at(depth).end(), sortMovePairs);

        Line localBestLine;

        /// Iterate through all moves
        for(auto& [info, move]: moves.at(depth)) {
            auto [nextBoard, nextState] = forkBoard(board, state, move);
            auto [eval, line] = quiescenceSearch(nextBoard, nextState, depth + 1,  -beta,  -alpha);
            eval = -eval;

            if (eval >= beta) {
                line.push_back(move);
                return { beta, line };
            }
            if (eval > alpha) {
                alpha = eval;
                line.push_back(move);
                localBestLine = line;
            }
        }

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
std::vector<Move> EngineMC::bestMoves{};
TranspositionTable EngineMC::trTable{};
RepetitionTable EngineMC::repTable{};
Move EngineMC::bestMove{};
Move EngineMC::priorityMove{};

#endif //DORY_ENGINE_H
