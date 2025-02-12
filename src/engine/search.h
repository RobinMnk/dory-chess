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

namespace Dory {

    using Line = std::vector<Move>;
    using Result = std::pair<int, Line>;

    namespace Search {

        struct WeightedMove {
            Move move{};
            int weight{0};

            bool operator>(const WeightedMove& other) const { return weight > other.weight; }
        };

//        class MoveList {
//            const MoveOrderer* moveOrderer;
//            PinData pd{};
//            size_t depth, ix{0};
//            std::array<WeightedMove, 256> moves;
//
//        public:
//            constexpr explicit MoveList(size_t dpt, const MoveOrderer* orderer) : moveOrderer{orderer}, depth{dpt} {}
//
//            [[nodiscard]] PinData& getPinData() {
//                return pd;
//            }
//
//            [[nodiscard]] bool empty() const {
//                return ix == 0;
//            }
//
//            [[nodiscard]] auto begin() const {
//                return moves.begin();
//            }
//
//            [[nodiscard]] auto end() const {
//                return moves.begin() + ix;
//            }
//
//            void sort() {
//                std::sort(moves.begin(), moves.begin() + ix,
//                  [](const WeightedMove& a, const WeightedMove& b) { return a.weight > b.weight; }
//                );
//            }
//
//            void reset() {
//                ix = 0;
//            }
//
//            template<bool whiteToMove,  Piece_t piece, Flag_t flags>
//            void nextMove(const Board& board, BB from, BB to) {
//                moves.at(ix++) = {
//                    createMoveFromBB(from, to, piece, flags),
//                    moveOrderer->moveHeuristic<whiteToMove, piece, flags>(board, from, to, pd, depth)
//                };
//            }
//        };
//
//        template<size_t depth>
//        constexpr std::array<MoveList, depth> buildCollectors(const MoveOrderer* orderer) {
//            return [orderer]<size_t... Is>(std::index_sequence<Is...>) {
//                return std::array<MoveList, depth>{MoveList(Is, orderer)... };
//            }(std::make_index_sequence<depth>{});
//        }
//
//        template<size_t maxDepth>
//        class MoveStorage {
//            std::array<MoveList, maxDepth> container;
//
//        public:
//            explicit MoveStorage(const MoveOrderer* moveOrderer) : container{buildCollectors<maxDepth>(moveOrderer)} {}
//
//            template<bool whiteToMove>
//            const PinData& loadClh(const Board& board, size_t depth) {
//                container.at(depth).reset();
//                CheckLogicHandler::reload<whiteToMove>(board, container.at(depth).getPinData());
//                return container.at(depth).getPinData();
//            }
//
//            template<bool whiteToMove, GenerationConfig config=GC_DEFAULT>
//            void generate(const Board& board, size_t depth) {
//                if constexpr (config.reloadClh) {
//                    loadClh<whiteToMove>(board, depth);
//                }
//                MoveCollectors::template generateMoves<MoveList, whiteToMove, config>(&container.at(depth), board, container.at(depth).getPinData());
//            }
//
//            [[nodiscard]] const PinData& pd(size_t depth) const {
//                return container.at(depth).getPinData();
//            }
//
//            [[nodiscard]] auto begin(size_t depth) const {
//                return container.at(depth).begin();
//            }
//
//            [[nodiscard]] auto end(size_t depth) const {
//                return container.at(depth).end();
//            }
//
//            [[nodiscard]] bool empty(size_t depth) const {
//                return container.at(depth).empty();
//            }
//
//            void sort(size_t depth) {
//                container.at(depth).sort();
//            }
//
//            void reset() {
//                for(auto it: container) it.reset();
//            }
//
//            MoveList list(size_t depth) {
//                return container.at(depth);
//            }
//        };

//        template<size_t stacksize, size_t maxdepth>
        class MoveContainer {
        public:
            std::array<WeightedMove, 2048> moves{};
            std::array<size_t, 128> starts{};
            const MoveOrderer* moveOrderer;
            size_t currentDepth{}, ix{};
            PinData pd;

            explicit MoveContainer(const MoveOrderer* mO) : moveOrderer{mO} {}

            template<bool whiteToMove>
            const PinData& loadClh(const Board& board) {
                CheckLogicHandler::reload<whiteToMove>(board, pd);
                return pd;
            }

            template<bool whiteToMove, GenerationConfig config=GC_DEFAULT>
            void generate(const Board& board, size_t depth) {
                if constexpr (config.reloadClh) {
                    CheckLogicHandler::reload<whiteToMove>(board, pd);
                }
                currentDepth = depth;
                ix = starts[depth];
                MoveCollectors::template generateMoves<MoveContainer, whiteToMove, config>(this, board, pd);
                starts[depth+1] = ix;
            }

            template<bool whiteToMove,  Piece_t piece, Flag_t flags>
            void nextMove(const Board& board, BB from, BB to) {
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
        const int NUM_PV_NODES = 2;
        const int NUM_FULL_DEPTH_NODES = 4;

        class Searcher {
            TranspositionTable trTable{};
            RepetitionTable repTable{};
            MoveOrderer moveOrderer{};
            MoveContainer moveContainer;

        public:
            BB nodesSearched{0}, tableLookups{0};
            Move bestMove;

            Searcher() : moveContainer{&moveOrderer} {}

            template<bool whiteToMove>
            Result iterativeDeepening(const Board &board, int maxDepth = MAX_ITER_DEPTH);

            void reset() {
                trTable.reset();
//                repTable.reset();
                moveOrderer.reset();
                moveContainer.reset();
                nodesSearched = tableLookups = 0;
            }

            size_t trTableSizeKb() const { return trTable.size(); }

            size_t trTableSizeMb() const { return trTable.size() / 1024; }

        private:

            template<bool whiteToMove, bool topLevel>
            Result negamax(const Board &board, int depth, int alpha, int beta, int maxDepth);

            template<bool whiteToMove>
            Result quiescenceSearch(const Board &board, int depth, int alpha, int beta);

        }; // class Searcher

        template<bool whiteToMove>
        Result Searcher::iterativeDeepening(const Board &board, int maxDepth) {
//            reset();
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

//                std::cout << "Line for depth " << depth << std::endl;
//                std::cout << "Depth " << depth << " -> ";
                Utils::printLine(bestLine, bestEval);

                depth++;
            }
            return {bestEval, bestLine};
        }

        template<bool whiteToMove, bool topLevel>
        Result Searcher::negamax(const Board &board, int depth, int alpha, int beta, int maxDepth) {

            size_t boardHash = Zobrist::hash<whiteToMove>(board);

//            if(depth > 100) {
//                return quiescenceSearch<whiteToMove>(board, depth, alpha, beta);
//            }

            /// Check for Threefold-Repetition
            if (repTable.check(boardHash)) {
                return {0, {}};
            }

            /// Lookup position in table
            int origAlpha = alpha;
            auto [ttEntry, resultValid] = trTable.lookup(boardHash, alpha, beta, maxDepth - depth);
            if (resultValid) {
                tableLookups++;
                return {ttEntry.value, {}};
            }

            /// Switch to Quiescence Search
            const PinData& pd = moveContainer.loadClh<whiteToMove>(board);
            bool inCheck = pd.inCheck();

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
            moveContainer.generate<whiteToMove, GC_DEFAULT_NO_CLH>(board, depth);

            /// Check for Checkmate / Stalemate
            // No legal moves available
            if (moveContainer.empty(depth)) {
                if (inCheck) {
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

            moveContainer.sort(depth);

//            for (int i = 0; i < depth; i++)
//                std::cout << "   ";
//            std::cout << depth << ":  " << Utils::moveNameShortNotation(priorityMove) << std::endl;

//            for(WeightedMove wm: moveContainer.list(depth)) {
//                for (int i = 0; i < depth; i++)
//                    std::cout << "   ";
//                std::cout << depth << ":  " << Utils::moveNameShortNotation(wm.move) << std::endl;
//            }

//                moveOrderer.sort(moves.at(depth), moveIndices.at(depth));



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
            for(auto it = moveContainer.begin(depth); it != moveContainer.end(depth); ++it) {
                Move move = (*it).move;

                repTable.insert(boardHash);

                nextBoard = board.fork<whiteToMove>(move);

                /// Principal Variation Search
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
        Result Searcher::quiescenceSearch(const Board &board, int depth, int alpha, int beta) {
            /// Recursion Base Case: Max Depth reached -> return heuristic position eval
            int standPat = evaluation::evaluatePosition<whiteToMove>(board);

//            if(depth > 30) {
//                 return {alpha, {}};
//            }

            if (standPat >= beta) {
                nodesSearched++;
                return {beta, {}};
            }
            if (alpha < standPat) {
                alpha = standPat;
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
                nodesSearched++;
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
                    nodesSearched++;
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