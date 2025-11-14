//
// Created by Robin on 24.08.2023.
//

#ifndef DORY_SEARCH_H
#define DORY_SEARCH_H

#include "evaluation.h"
#include "../utils/utils.h"
#include "../utils/zobrist.h"
#include "../core/movegen.h"
#include "../core/movecollectors.h"
#include "moveordering.h"
#include "tables.h"
#include "../utils/timer.h"

namespace Dory {

    using Line = std::vector<Move>;
    struct Result {
        int eval{};
        Line line{};
    };

    namespace Search {

        struct WeightedMove {
            Move move{};
            int weight{0};

            bool operator>(const WeightedMove& other) const { return weight > other.weight; }
        };

//        template<size_t stacksize, size_t maxdepth>
        class MoveContainer {
            std::array<WeightedMove, 2048> moves{};
            std::array<size_t, 128> starts{};
            const MoveOrderer* moveOrderer;
            size_t currentDepth{}, ix{};
            PinData pd;

        public:
            explicit MoveContainer(const MoveOrderer* mO) : moveOrderer{mO} {}

            template<bool whiteToMove>
            inline const PinData& loadClh(Board& board) {
                CheckLogicHandler::reload<whiteToMove>(board, pd);
                return pd;
            }

            template<bool whiteToMove, GenerationConfig config=GC_DEFAULT>
            void generate(Board& board, size_t depth) {
                if constexpr (config.reloadClh) {
                    CheckLogicHandler::reload<whiteToMove>(board, pd);
                }
                currentDepth = depth;
                ix = starts[depth];
                MoveCollectors::template generateMoves<MoveContainer, whiteToMove, config>(this, board, pd);
                starts[depth+1] = ix;
            }

            template<bool whiteToMove,  Piece_t piece, Flag_t flags>
            inline void nextMove(Board& board, BB from, BB to) {
                moves.at(ix++) = {
                    createMoveFromBB(from, to, piece, flags),
                    moveOrderer->moveHeuristic<whiteToMove, piece, flags>(board, from, to, pd, currentDepth)
                };
            }

            [[nodiscard]] auto begin(size_t depth) const { return moves.begin() + starts[depth]; }
            [[nodiscard]] auto end(size_t depth) const { return moves.begin() + starts[depth+1]; }
            [[nodiscard]] bool empty(size_t depth) const { return starts[depth] == starts[depth+1]; }

            // Modifiers
            void sort(size_t depth) { std::sort(moves.begin() + starts[depth], moves.begin() + starts[depth+1], std::greater<>()); }
            void reset() { starts.fill(0); }
        };

        const int MAX_ITER_DEPTH = 6;
        const int MAX_WINDOW_INCREASES = 2;
        const int ASP_WINDOW_SIZE = 20;
        const int NUM_PV_NODES = 2;
        const int NUM_FULL_DEPTH_NODES = 4;

        class Searcher {
            TranspositionTable trTable{};
            RepetitionTable repTable{};
            MoveOrderer moveOrderer{};
            MoveContainer moveContainer{&moveOrderer};

        public:
            BB nodesSearched{0}, tableLookups{0};
            Move bestMove;

            template<bool whiteToMove>
            Result iterativeDeepening(Board &board, int maxDepth = MAX_ITER_DEPTH);

            void reset() {
                trTable.reset();
                repTable.reset();
                moveOrderer.reset();
                moveContainer.reset();
                nodesSearched = tableLookups = 0;
            }

            [[nodiscard]] size_t trTableSizeKb() const { return trTable.size(); }

            [[nodiscard]] size_t trTableSizeMb() const { return trTable.size() / 1024; }

        private:

            template<bool whiteToMove, bool topLevel>
            Result negamax(Board &board, int depth, int alpha, int beta, int maxDepth);

            template<bool whiteToMove>
            Result quiescenceSearch(Board &board, int depth, int alpha, int beta);

        }; // class Searcher

        bool isMateEval(int eval) {
            return eval > INF - 50 || eval < -(INF - 50);
        }

        template<bool whiteToMove>
        Result Searcher::iterativeDeepening(Board &board, int maxDepth) {
            Result bestResult{};
            int alpha, beta;
            reset();

            Timer t;
            t.start();

            for (int depth = 1; depth <= maxDepth; depth++) {
                int window = ASP_WINDOW_SIZE;
                alpha = (depth == 1) ? -INF : bestResult.eval - window;
                beta  = (depth == 1) ?  INF : bestResult.eval + window;

                std::cout << "Searching Depth " << depth << "    (" << alpha << " / " << beta << ")" << std::endl;

                int windowIncreases = MAX_WINDOW_INCREASES;
                Result result{};
                bool doFullSearch = false;

                while (windowIncreases--) {
                    result = negamax<whiteToMove, true>(board, 0, alpha, beta, depth);

                    if (isMateEval(result.eval)) break;

                    if (result.eval <= alpha) {
                        alpha -= window;
                        doFullSearch = true;
                    } else if (result.eval >= beta) {
                        beta += window;
                        doFullSearch = true;
                    } else {
                        doFullSearch = false;
                        break; // within window
                    }
                    window *= 2;
                }

                if (doFullSearch) {
                    result = negamax<whiteToMove, true>(board, 0, -INF, INF, depth);
                }

                bestResult = std::move(result);
                Utils::printLine(bestResult.line, bestResult.eval);

                auto s = t.timeSeconds();
                std::cout << (static_cast<double>(nodesSearched) / 1000000) / s << " M nodes / second\t\t[" << nodesSearched << " nodes in " << s << " sec]\n" << std::endl;
            }

            return bestResult;
        }

        template<bool whiteToMove, bool topLevel>
        Result Searcher::negamax(Board &board, int depth, int alpha, int beta, int maxDepth) {
            const uint64_t boardHash = Zobrist::hash<whiteToMove>(board);
            nodesSearched++;

            /// Check for Threefold-Repetition
            if (repTable.check(boardHash)) {
                return {0, {}};
            }

            /// Lookup position in table
            int origAlpha = alpha;
            int remainingDepth = maxDepth - depth;
            auto [ttEntry, resultValid] = trTable.lookup(boardHash, alpha, beta, remainingDepth);
            if (resultValid) {
                tableLookups++;
                return {ttEntry.value, {}};
            }

            /// Switch to Quiescence Search
            const PinData& pd = moveContainer.loadClh<whiteToMove>(board);
            bool inCheck = pd.inCheck();

            if (!inCheck && depth >= maxDepth) {
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
            moveContainer.generate<whiteToMove, GC_DEFAULT_NO_CLH>(board, depth);

            /// Check for Checkmate / Stalemate
            // No legal moves available
            if (moveContainer.empty(depth)) {
                if (inCheck) {
                    // Checkmate!
                    int eval = -(INF - depth);
                    trTable.insert(boardHash, eval, NULLMOVE, remainingDepth, origAlpha, beta);
                    return {eval, {}};
                } else {
                    // Stalemate!
                    trTable.insert(boardHash, 0, NULLMOVE, remainingDepth, origAlpha, beta);
                    return {0, {}};
                }
            }

            moveContainer.sort(depth);

            // Iterate all moves
            Line localBestLine;
            Move localBestMove;
//            Board nextBoard;
            int bestEval = -INF;

//            int eval;
//            Line line;

            // Search Extensions
            int mdpt = maxDepth;
            if (inCheck) mdpt++; // very important!
//                else if (maxDepth - depth > 2) rdpt--;
//            int rdpt = maxDepth;


            /// Iterate through all moves
            int moveIx = 0;
            for(auto it = moveContainer.begin(depth); it != moveContainer.end(depth); ++it) {
                Move move = (*it).move;
                bool isCapture = board.isCapture<whiteToMove>(move);

                repTable.push(boardHash);
                RestoreInfo ri = board.makeMove<whiteToMove>(move);

//                int ext = 0;
//                if(inCheck || move.isPromotion()) ext = 1;
////                else {
////                    if (depth >= 3 && moveIx >= 12 && !isCapture)
////                        ext = -1;
////                }
//                int mdpt = maxDepth + ext;

                int eval;
                Line line;

                // Principal Variation Search
                if (moveIx == 0) {
                    // First move: full window search
                    auto [ev, ln] = negamax<!whiteToMove, false>(board, depth + 1, -beta, -alpha, mdpt);
                    eval = -ev;
                    line = ln;
                } else {
                    // Late move: null-window search first
                    auto [ev, ln] = negamax<!whiteToMove, false>(board, depth + 1, -alpha - 1, -alpha, mdpt);
                    int tempEval = -ev;

                    if (tempEval > alpha && tempEval < beta) {
                        // Fail-high → full re-search needed
                        auto [ev2, ln2] = negamax<!whiteToMove, false>(board, depth + 1, -beta, -alpha, mdpt);
                        eval = -ev2;
                        line = ln2;
                    } else {
                        eval = tempEval;
                        line = ln;
                    }
                }

                board.unmakeMove<whiteToMove>(move, ri);
                repTable.pop();

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
                    if(!isCapture)
                        moveOrderer.addKillerMove(move, depth);
                    break;
                }

                moveIx++;
            } // end iterate moves

            /// Save to lookup table
            trTable.insert(boardHash, bestEval, localBestMove, remainingDepth, origAlpha, beta);

            return {bestEval, localBestLine};
        }

        template<bool whiteToMove>
        Result Searcher::quiescenceSearch(Board &board, int depth, int alpha, int beta) {
            nodesSearched++;

            /// Recursion Base Case: Max Depth reached -> return heuristic position eval
            int standPat = evaluation::evaluatePosition<whiteToMove>(board);

            if (standPat >= beta) {
                return {beta, {}};
            }
            if (alpha < standPat) {
                alpha = standPat;
            }

            if(depth > 30) {
                 return {alpha, {}};
            }

            // Reload CLH
            const PinData& pd = moveContainer.loadClh<whiteToMove>(board);
            bool inCheck = pd.inCheck();

            if (inCheck) {
                // if in check, any legal move is considered
                moveContainer.generate<whiteToMove, GC_DEFAULT_NO_CLH>(board, depth);
            } else {
                // if not in check, consider only captures
                moveContainer.generate<whiteToMove, GC_QUIESCENCE_NO_CLH>(board, depth);
            }

            if (moveContainer.empty(depth)) {
                return {alpha, {}};
            }

            moveContainer.sort(depth);

            Line localBestLine;
            Board nextBoard;

            /// Iterate through all moves
            for(auto cit = moveContainer.begin(depth); cit != moveContainer.end(depth); ++cit) {
                Move move = (*cit).move;

                nextBoard = board.fork<whiteToMove>(move);
                auto [eval, line] = quiescenceSearch<!whiteToMove>(nextBoard, depth + 1, -beta, -alpha);
                eval = -eval;

//            for (int i = 0; i < depth; i++)
//                std::cout << "   ";
//            std::cout << depth << " : " << Utils::moveNameShortNotation(move) << "  " << eval << std::endl;

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

            return {alpha, localBestLine};
        }


    } // namespace Search

} // namespace Dory

#endif //DORY_SEARCH_H