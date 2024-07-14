//
// Created by Robin on 24.08.2023.
//

#ifndef DORY_SEARCH_H
#define DORY_SEARCH_H

#include "evaluation.h"
#include "../utils/utils.h"
#include "../utils/zobrist.h"
#include "../core/movegen.h"
#include "moveordering.h"
#include "tables.h"

namespace Dory::Search {

    unsigned int NUM_LINES = 1;
    const int BEST_MOVE_MARGIN = 10;
    const int MAX_ITER_DEPTH = 6;
    const int MAX_WINDOW_INCREASES = 3;

    using Line = std::vector<Move>;
    using NMR = std::pair<int, Line>;


    template<bool whiteToMove>
    int subjectiveEval(int eval) {
        if constexpr (whiteToMove) return eval;
        else return -eval;
    }

    class Searcher {
        static Move priorityMove;
        static std::array<std::vector<std::pair<int, Move>>, 128> moves;
        static int currentDepth;
    public:
        static TranspositionTable trTable;
        static RepetitionTable repTable;
        static std::vector<std::pair<Line, int>> bestLines;
        static BB nodesSearched;
        static Move bestMove;
        static std::vector<Move> bestMoves;

        template<bool whiteToMove>
        static NMR iterativeDeepening(const Board &board, int maxDepth = MAX_ITER_DEPTH) {
            reset();
//        repTable.reset();
            Line bestLine;
            int bestEval = -INF, window = 20, windowIncreases = 0;
            int alpha = -INF, beta = INF;

            for (int depth = 1; depth <= maxDepth;) {
//            std::cout << "Searching Depth " << depth << "    (" << alpha << " / " << beta << ")" << std::endl;
                auto [eval, line] = searchDepth<whiteToMove>(board, depth, alpha, beta);

                /// Aspiration Window
                if (eval <= alpha || eval >= beta) {
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
            return {bestEval, bestLine};
        }

        template<bool whiteToMove>
        static NMR searchDepth(const Board &board, int depth, int alpha = -INF, int beta = INF) {
//        maxDepth = depth;
            bestLines.clear();
            return negamax<whiteToMove, true>(board, 1, alpha, beta, depth);
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

        template<bool whiteToMove, bool topLevel>
        static NMR negamax(const Board &board, int depth, int alpha, int beta, int maxDepth) {

            size_t boardHash = Zobrist::hash<whiteToMove>(board);

            /// Check for Threefold-Repetition
            if (repTable.check(boardHash)) {
                return {0, {}};
            }

            /// Lookup position in table
            int origAlpha = alpha;
            auto [ttEntry, resultValid] = trTable.lookup(boardHash, alpha, beta, maxDepth - depth);
            if (resultValid) {
                trTable.lookups++;
                return {ttEntry.value, {}};
            }

            /// Switch to Quiescence Search
            if (depth > maxDepth) {
//            nodesSearched++;
//            int eval = Evaluation::evaluatePosition(board, state);
//            trTable.insert(boardHash, eval, NULLMOVE, maxDepth - depth, origAlpha, beta);
//            return { eval, {} };

//                return negamax<false, true>(board, state, depth, alpha, beta);

                return quiescenceSearch<whiteToMove>(board, depth, alpha, beta);
            }


            /// Generate legal moves

            // Set move that is searched first
            if constexpr (topLevel) {
                priorityMove = bestMove;
            } else {
                // ttEntry.move may be NULLMOVE, but that does not hurt us
                priorityMove = ttEntry.move;
            }

            // Setup variables before generating legal moves
            moves.at(depth).clear();
            currentDepth = depth;
            MoveGenerator<Searcher>::template generate<whiteToMove>(board);

            // No legal moves available
            if (moves.at(depth).empty()) {
                if (MoveGenerator<Searcher>::pd->inCheck()) {
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

//        for (int i = 0; i < depth; i++)
//            std::cout << "   ";
//        std::cout << depth << ":  " << Utils::moveNameShortNotation(priorityMove) << std::endl;

            std::sort(moves.at(depth).begin(), moves.at(depth).end(), sortMovePairs);


            // Iterate all moves
            Line localBestLine;
            Move localBestMove;
            int bestEval = -INF;

            int isPV = 4;
            int eval;
            Line line;

            // Search Extensions
            int mdpt = maxDepth;
            if (MoveGenerator<Searcher>::pd->inCheck()) mdpt++;

//            if(depth + 1 == maxDepth && MoveGenerator<Searcher>::pd->inCheck())
//                mdpt++;
//            maxDepth++;

//        if constexpr (topLevel) {
//            for(auto& [_, m]: moves.at(depth)) {
//                std::cout << Utils::moveNameNotation(m) << ",  ";
//            }
//            std::cout << std::endl;
//        }

            /// Iterate through all moves
            for (auto &[info, move]: moves.at(depth)) {

                repTable.insert(boardHash);

                Board nextBoard = board.fork<whiteToMove>(move);

                /// Pricipal Variation Search
                if (isPV > 0) {
                    auto [ev, ln] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -beta, -alpha, mdpt);
                    eval = -ev;
                    line = ln;
                } else {
                    int redMdpt = maxDepth;
//                if constexpr (topLevel)
//                    if(mdpt - depth >= 3 && !board.isCapture<whiteToMove>(move) && mdpt == maxDepth && !MoveGenerator<Searcher>::pd->inCheck()) redMdpt--;
                    auto [ev, ln] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -alpha - 1, -alpha, redMdpt);
                    if (ev < -alpha && ev > -beta) {
                        auto [ev2, ln2] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -beta, -alpha,
                                                                       redMdpt);
                        eval = -ev2;
                        line = ln2;
                    } else {
                        eval = -ev;
                        line = ln;
                    }
                }
                isPV--;


                repTable.remove(boardHash);

//            if(Zobrist::hash<whiteToMove>(board) == 335140086) {
//                Line l = std::vector<Move>{move};
//                std::cout << info << " for " << whiteToMove << " -> ";
//                Utils::printLine(l,  info);
//            }

//            for (int i = 0; i < depth; i++)
//                std::cout << "   ";
//            std::cout << depth << " : " << Utils::moveNameShortNotation(move) << "  " << eval << std::endl;

//            if constexpr (topLevel) {
//                Line l = std::vector<Move>{move};
//                Utils::printLine(l, info);
//            }

                if (eval > alpha)
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

            if (bestEval == -INF) {
                std::cout << "ERROR!!!" << std::endl;
            }

            /// Save to lookup table
            trTable.insert(boardHash, bestEval, localBestMove, maxDepth - depth, origAlpha, beta);

            return {bestEval, localBestLine};
        }

        template<bool whiteToMove>
        static NMR quiescenceSearch(const Board &board, int depth, int alpha, int beta) {
            /// Recursion Base Case: Max Depth reached -> return heuristic position eval
            int standPat = Evaluation::evaluatePosition<whiteToMove>(board);

            if (standPat >= beta) {
                nodesSearched++;
                return {beta, {}};
            }
            if (alpha < standPat) {
                alpha = standPat;
            }

            moves.at(depth).clear();
            currentDepth = depth;

            CheckLogicHandler::reload<whiteToMove>(board, MoveGenerator<Searcher, true>::pd);

            if (MoveGenerator<Searcher, true>::pd->inCheck()) {
                *MoveGenerator<Searcher>::pd = *MoveGenerator<Searcher, true>::pd;
                MoveGenerator<Searcher>::template generate<whiteToMove, false>(board);
            } else {
                MoveGenerator<Searcher, true>::template generate<whiteToMove, false>(board);
            }

            if (moves.at(depth).empty()) {
                nodesSearched++;
                return {alpha, {}};
            }

            std::sort(moves.at(depth).begin(), moves.at(depth).end(), sortMovePairs);

            Line localBestLine;

            /// Iterate through all moves
            for (auto &[info, move]: moves.at(depth)) {

                Board nextBoard = board.fork<whiteToMove>(move);
                auto [eval, line] = quiescenceSearch<!whiteToMove>(nextBoard, depth + 1, -beta, -alpha);
                eval = -eval;

//            for (int i = 0; i < depth; i++)
//                std::cout << "   ";
//            std::cout << depth << " : " << Utils::moveNameShortNotation(move) << "  " << eval << std::endl;

                if (eval >= beta) {
                    line.push_back(move);
                    nodesSearched++;
                    return {beta, line};
                }
                if (eval > alpha) {
                    alpha = eval;
                    line.push_back(move);
                    localBestLine = line;
                }
            }

            return {alpha, localBestLine};
        }

        template<bool whiteToMove, Piece_t piece, Flag_t flags = MOVEFLAG_Silent>
        static void registerMove(const Board &board, BB from, BB to) {
            // TODO reload checklogichandler and pass PinData to move_info
            moves[currentDepth].emplace_back(
                    move_heuristic<whiteToMove, piece, flags>(board, from, to, MoveGenerator<Searcher>::pd, priorityMove),
                    createMoveFromBB(from, to, piece, flags)
            );
        }

        friend class MoveGenerator<Searcher, true>;
        friend class MoveGenerator<Searcher, false>;
    };

    std::array<std::vector<std::pair<int, Move>>, 128> Searcher::moves{};
    int Searcher::currentDepth{0};
//int Searcher::maxDepth{0};
    BB Searcher::nodesSearched{0};
    std::vector<std::pair<Line, int>> Searcher::bestLines{};
    std::vector<Move> Searcher::bestMoves{};
    TranspositionTable Searcher::trTable{};
    RepetitionTable Searcher::repTable{};
    Move Searcher::bestMove{};
    Move Searcher::priorityMove{};

} // namespace Dory

#endif //DORY_SEARCH_H
