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
        const int MAX_WINDOW_INCREASES = 2;
        const int NUM_PV_NODES = 2;
        const int NUM_FULL_DEPTH_NODES = 4;

        class Searcher {
            static std::array<std::array<std::pair<float, Move>, 256>, 128> moves;
            static std::array<unsigned int, 128> moveIndices;
            static int currentDepth;
        public:
            static TranspositionTable trTable;
            static RepetitionTable repTable;
            static MoveOrderer moveOrderer;

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
                    std::cout << "Searching Depth " << depth << "    (" << alpha << " / " << beta << ")" << std::endl;
                    auto [eval, line] = negamax<whiteToMove, true>(board, 1, alpha, beta, depth);

                    /// Aspiration Window
                    if (eval <= alpha || eval >= beta) {
                        if (windowIncreases++ >= MAX_WINDOW_INCREASES) {
                            alpha = -INF;
                            beta = INF;
                        } else {
                            window *= 2;
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

//                    std::cout << "Line for depth " << depth << std::endl;
//                    std::cout << "Depth " << depth << " -> ";
                    Utils::printLine(bestLine, bestEval);

                    depth++;
                }
                return {bestEval, bestLine};
            }

            static void reset() {
                searchResults.reset();
                trTable.reset();
//                repTable.reset();
                moveOrderer.reset();
                moveIndices.fill(0);
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
                bool inCheck = MoveGenerator<Searcher>::pd->inCheck();

                if (!inCheck && depth > maxDepth) {
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
                moveIndices.at(depth) = 0;
                currentDepth = depth;
                generateMoves<Searcher, whiteToMove, false>(board);

                /// Check for Checkmate / Stalemate
                // No legal moves available
                if (moveIndices.at(depth) == 0) {
                    if (inCheck) {
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

                moveOrderer.sort(moves.at(depth), moveIndices.at(depth));

//                for(unsigned int i = 0; i < moves.at(depth).size(); i++) {
//                    std::cout << Utils::moveNameShortNotation(moves[depth][i].second) << "  : " << moves[depth][i].first << std::endl;
//                }


                // Iterate all moves
                Line localBestLine;
                Move localBestMove;
                Board nextBoard;
                int bestEval = -INF;

                int eval;
                Line line;

                // Search Extensions
                int mdpt = maxDepth;
                if (inCheck) mdpt++; // very important!
//                else if (maxDepth - depth > 2) rdpt--;
                int rdpt = maxDepth;


                /// Iterate through all moves
                int moveIx = 0;
                for(auto cit = moves.at(depth).begin(); cit != moves.at(depth).begin() + moveIndices.at(depth); ++cit) {
                    auto& [info, move] = *cit;

                    repTable.insert(boardHash);

                    nextBoard = board.fork<whiteToMove>(move);

                    /// Pricipal Variation Search
                    bool doFullSearch = true;
                    if(maxDepth - depth > 2 && moveIx == NUM_FULL_DEPTH_NODES) rdpt -= 2;
                    if (moveIx >= NUM_PV_NODES && !board.isCapture<whiteToMove>(move) && !inCheck) {
                        // not part of the principal variation and no capture -> try a zero-window search
                        auto [ev, ln] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -alpha - 1, -alpha, rdpt);
                        if (ev < -alpha && ev > -beta) {
                            // zero-window assumption failed -> needs full search
                        } else {
                            eval = -ev;
                            line = ln;
                            doFullSearch = false;
                        }
                    }

                    if(doFullSearch) {
                        auto [ev, ln] = negamax<!whiteToMove, false>(nextBoard, depth + 1, -beta, -alpha, mdpt);
                        eval = -ev;
                        line = ln;
                    }

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
                            bestMove = move;
                        }
                    }

                    if (alpha >= beta) {
                        if(!board.isCapture<whiteToMove>(move))
                            moveOrderer.addKillerMove(move, depth);
                        break;
                    }

                    moveIx++;
                } // end iterate moves

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
                int standPat = evaluation::evaluatePosition<whiteToMove>(board);

                if (standPat >= beta) {
                    searchResults.nodesSearched++;
                    return {beta, {}};
                }
                if (alpha < standPat) {
                    alpha = standPat;
                }

                moveIndices.at(depth) = 0;
                currentDepth = depth;

                // Reload CLH
                CheckLogicHandler::reload<whiteToMove>(board, MoveGenerator<Searcher, true>::pd);

                if (MoveGenerator<Searcher, true>::pd->inCheck()) {
                    // if in check, any legal move is considered
                    *MoveGenerator<Searcher>::pd = *MoveGenerator<Searcher, true>::pd;
                    generateMoves<Searcher, whiteToMove, false>(board);
                } else {
                    // if not in check, consider only captures
                    generateMoves<Searcher, whiteToMove, false, true>(board);
                }

                if (moveIndices.at(depth) == 0) {
                    searchResults.nodesSearched++;
                    return {alpha, {}};
                }

                moveOrderer.sort(moves.at(depth), moveIndices.at(depth));

                Line localBestLine;
                Board nextBoard;

                /// Iterate through all moves
                for(auto cit = moves.at(depth).begin(); cit != moves.at(depth).begin() + moveIndices.at(depth); ++cit) {
                    auto& [info, move] = *cit;

                    nextBoard = board.fork<whiteToMove>(move);
                    auto [eval, line] = quiescenceSearch<!whiteToMove>(nextBoard, depth + 1, -beta, -alpha);
                    eval = -eval;

//            for (int i = 0; i < depth; i++)
//                std::cout << "   ";
//            std::cout << depth << " : " << Utils::moveNameShortNotation(move) << "  " << eval << std::endl;

                    if (eval >= beta) {
                        line.push_back(move);
                        searchResults.nodesSearched++;
                        return { beta, line };
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
                moves[currentDepth].at(moveIndices.at(currentDepth)++)
                    = std::make_pair(
                        moveOrderer.moveHeuristic<whiteToMove, piece, flags>(board, from, to, MoveGenerator<Searcher>::pd, currentDepth),
                        createMoveFromBB(from, to, piece, flags)
                    );
            }

            friend class MoveGenerator<Searcher, true>;
            friend class MoveGenerator<Searcher, false>;
        };

        std::array<std::array<std::pair<float, Move>, 256>, 128> Searcher::moves{};
        std::array<unsigned int, 128> Searcher::moveIndices{};
        int Searcher::currentDepth{0};
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
