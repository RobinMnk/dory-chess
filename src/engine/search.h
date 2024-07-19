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

namespace Dory {

    using Line = std::vector<Move>;
    using Result = std::pair<int, Line>;

    struct SearchResults {
        BB nodesSearched{0}, tableLookups{0};
        size_t trTableSizeKb();
        size_t trTableSizeMb();

        void reset() {
            nodesSearched = tableLookups = 0;
        }
    };

    static SearchResults searchResults;

    namespace Search {

        unsigned int NUM_LINES = 1;
        const int BEST_MOVE_MARGIN = 10;
        const int MAX_ITER_DEPTH = 6;
        const int MAX_WINDOW_INCREASES = 3;

        class Searcher {
            static std::array<std::vector<std::pair<float, Move>>, 128> moves; // Todo: make fixed-size array?
            static int currentDepth;
        public:
            static TranspositionTable trTable;
            static RepetitionTable repTable;
            static MoveOrderer moveOrderer;

            static std::vector<std::pair<Line, int>> bestLines;
            static Move bestMove;
            static std::vector<Move> bestMoves;

            template<bool whiteToMove>
            static Result iterativeDeepening(const Board &board, int maxDepth = MAX_ITER_DEPTH) {
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
            static Result searchDepth(const Board &board, int depth, int alpha = -INF, int beta = INF) {
//        maxDepth = depth;
                bestLines.clear();
                return negamax<whiteToMove, true>(board, 1, alpha, beta, depth);
            }

            static bool sortMovePairs(const std::pair<float, Move> &a, const std::pair<float, Move> &b) {
                return a.first > b.first;
            }

//        static std::vector<Move> topLevelLegalMoves() {
//            return moves[1];
//        }

            static void reset() {
                searchResults.reset();
                trTable.reset();
                repTable.reset();
//            moveOrderer.reset();
//        nodesSearched = 0;
            }

        private:

            template<bool whiteToMove, bool topLevel>
            static Result negamax(const Board &board, int depth, int alpha, int beta, int maxDepth) {

                size_t boardHash = Zobrist::hash<whiteToMove>(board);

                /// Check for Threefold-Repetition
                if (repTable.check(boardHash)) {
                    return {0, {}};
                }

                /// Lookup position in table
                int origAlpha = alpha;
                auto [ttEntry, resultValid] = trTable.lookup(boardHash, alpha, beta, maxDepth - depth);
                if (resultValid) {
                    searchResults.tableLookups++;
                    return {ttEntry.value, {}};
                }

                /// Switch to Quiescence Search

                CheckLogicHandler::reload<whiteToMove>(board, MoveGenerator<Searcher>::pd);

                if (!MoveGenerator<Searcher>::pd->inCheck() && depth > maxDepth) {
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
                    moveOrderer.priorityMove = bestMove;
                } else {
                    // ttEntry.move may be NULLMOVE, but that does not hurt us
                    moveOrderer.priorityMove = ttEntry.move;
                }

                // Setup variables before generating legal moves
                moves.at(depth).clear();
                currentDepth = depth;
                generateMoves<Searcher, whiteToMove, false>(board);

                // No legal moves available
                if (moves.at(depth).empty()) {
                    if (MoveGenerator<Searcher>::pd->inCheck()) {
                        // Checkmate!
                        searchResults.nodesSearched++;
                        int eval = -(INF - depth);
                        trTable.insert(boardHash, eval, NULLMOVE, maxDepth - depth, origAlpha, beta);
                        return {eval, {}};
                    } else {
                        // Stalemate!
                        searchResults.nodesSearched++;
                        trTable.insert(boardHash, 0, NULLMOVE, maxDepth - depth, origAlpha, beta);
                        return {0, {}};
                    }
                }

//        for (int i = 0; i < depth; i++)
//            std::cout << "   ";
//        std::cout << depth << ":  " << Utils::moveNameShortNotation(priorityMove) << std::endl;

                moveOrderer.sort(moves.at(depth));


                // Iterate all moves
                Line localBestLine;
                Move localBestMove;
                Board nextBoard;
                int bestEval = -INF;

                int isPV = 4;
                int eval;
                Line line;

                // Search Extensions
                int mdpt = maxDepth;
                if (MoveGenerator<Searcher>::pd->inCheck()) mdpt++; // apparently super important!

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
                for (auto [info, move]: moves.at(depth)) {

                    repTable.insert(boardHash);

                    nextBoard = board.fork<whiteToMove>(move);

                    /// Pricipal Variation Search
                    if (isPV > 0) {
                        auto [ev, ln] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -beta, -alpha, mdpt);
                        eval = -ev;
                        line = ln;
                    } else {
//                if constexpr (topLevel)
//                    if(mdpt - depth >= 3 && !board.isCapture<whiteToMove>(move) && mdpt == maxDepth && !MoveGenerator<Searcher>::pd->inCheck()) redMdpt--;
                        auto [ev, ln] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -alpha - 1, -alpha, mdpt);
                        if (ev < -alpha && ev > -beta) {
                            auto [ev2, ln2] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -beta, -alpha,
                                                                           maxDepth - 1);
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
//                    moveOrderer.addKillerMove(move, depth);
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
            static Result quiescenceSearch(const Board &board, int depth, int alpha, int beta) {
                /// Recursion Base Case: Max Depth reached -> return heuristic position eval
                int standPat = Evaluation::evaluatePosition<whiteToMove>(board);

                if (standPat >= beta) {
                    searchResults.nodesSearched++;
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
                    generateMoves<Searcher, whiteToMove, false>(board);
                } else {
                    generateMoves<Searcher, whiteToMove, false, true>(board);
                }

                if (moves.at(depth).empty()) {
                    searchResults.nodesSearched++;
                    return {alpha, {}};
                }

                moveOrderer.sort(moves.at(depth));

                Line localBestLine;
                Board nextBoard;

                /// Iterate through all moves
                for (auto [info, move]: moves.at(depth)) {

                    nextBoard = board.fork<whiteToMove>(move);
                    auto [eval, line] = quiescenceSearch<!whiteToMove>(nextBoard, depth + 1, -beta, -alpha);
                    eval = -eval;

//            for (int i = 0; i < depth; i++)
//                std::cout << "   ";
//            std::cout << depth << " : " << Utils::moveNameShortNotation(move) << "  " << eval << std::endl;

                    if (eval >= beta) {
                        line.push_back(move);
                        searchResults.nodesSearched++;
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
                        moveOrderer.moveHeuristic<whiteToMove, piece, flags>(board, from, to,
                                                                             MoveGenerator<Searcher>::pd),
                        createMoveFromBB(from, to, piece, flags)
                );
            }

            friend class MoveGenerator<Searcher, true>;

            friend class MoveGenerator<Searcher, false>;
        };

        std::array<std::vector<std::pair<float, Move>>, 128> Searcher::moves{};
        int Searcher::currentDepth{0};
//int Searcher::maxDepth{0};
        std::vector<std::pair<Line, int>> Searcher::bestLines{};
        std::vector<Move> Searcher::bestMoves{};
        TranspositionTable Searcher::trTable{};
        RepetitionTable Searcher::repTable{};
        MoveOrderer Searcher::moveOrderer{};
        Move Searcher::bestMove{};

    } // namespace Search

    size_t SearchResults::trTableSizeKb() {
        return Search::Searcher::trTable.size();
    }
    size_t SearchResults::trTableSizeMb() {
        return Search::Searcher::trTable.size() / 1024;
    }


} // namespace Dory

#endif //DORY_SEARCH_H
