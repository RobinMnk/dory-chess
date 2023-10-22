////
//// Created by robin on 24.08.2023.
////
//
//#ifndef DORY_ENGINE_H
//#define DORY_ENGINE_H
//
//#include <iostream>
//#include <stack>
//#include <vector>
//#include <memory>
//#include <thread>
//#include <unordered_map>
//#include "../movegen.h"
//#include "evaluation.h"
//#include "../utils.h"
//#include "../zobrist.h"
//
//float INF = 99999;
//
//int MAX_FORCING_MOVE_DEPTH = 3;
//int DEPTH_MARGIN_SIZE = 0;
//float BEST_MOVE_MARGIN = 0.05;
//size_t MAX_NUMBER_BEST_MOVES = 6;
//size_t MARGIN_THRESHOLD = 3;
//
//using NMR = std::pair<float, std::vector<Move>>;
//
//
//float subjectiveEval(float eval, State state) {
//    return state.whiteToMove ? eval : -eval;
//}
//
//bool sortMovePairs(const std::pair<float, Move> &a, const std::pair<float, Move> &b) {
//    return a.first < b.first;
//}
//
//const uint8_t TTFlagExact = 0, TTFlagLowerBound = 1, TTFlagUpperBound = 2;
//
//struct TTEntry {
//    float value;
//    uint8_t depth, flag;
//};
//
//class EngineMC {
//public:
////    template<State state>
////    static void start(const Board& board, int depth) {
////        std::jthread thr{[&board, &depth]{
////            beginEvaluation<state>(board, depth);
////        }};
////    }
//
//    static NMR beginEvaluation(const BoardPtr& board, const State state, int md) {
//        boundaryDepth = md;
//        maxDepth = md + DEPTH_MARGIN_SIZE;
//        bestMoves.clear();
//        lookup_table.clear();
//        nodesSearched = 0;
//        auto [ev, line] = negamax<true>(board, state, 1, -INF, INF);
//        bestMoves.clear();
//        bestMoves.push_back(line.back());
//        return {subjectiveEval(ev, state), line};
//    }
//
//    static float evaluation;
//    static std::vector<Move> bestMoves;
//    static BB nodesSearched, lookups;
//
//    static std::vector<std::pair<float, Move>> topLevelLegalMoves() {
//        return moves[0];
//    }
//private:
//    static std::array<std::vector<std::pair<float, Move>>, 32> moves;
//    static int maxDepth, boundaryDepth, currentDepth;
//    static std::unordered_map<BB, TTEntry> lookup_table;
//
//    static void saveToLookupTable(float eval, size_t boardHash, float alpha, float beta, int depth) {
//        uint8_t flag;
//        if (eval <= alpha)
//            flag = TTFlagUpperBound;
//        else if (eval >= beta)
//            flag = TTFlagLowerBound;
//        else flag = TTFlagExact;
//
//        TTEntry entry{ eval, static_cast<uint8_t>(depth), flag};
//        lookup_table.insert({ boardHash, entry });
//    }
//
//    static std::pair<NMR, bool> retrieveFromLookupTable(size_t boardHash, float alpha, float beta, int depth) {
//        auto res = lookup_table.find(boardHash);
//
//        if(res != lookup_table.end()) {
//            TTEntry entry = res->second;
//            if (entry.depth <= depth) {
//                lookups++;
//                if (entry.flag == TTFlagExact) {
//                    return {{ entry.value, {} }, true };
//                } else if (entry.flag == TTFlagLowerBound) {
//                    if (entry.value > alpha) alpha = entry.value;
//                } else if (entry.flag == TTFlagUpperBound) {
//                    if (entry.value < beta) beta = entry.value;
//                }
//
//                if (alpha >= beta) return { { entry.value, {} }, true };
//            }
//        }
//
//        return {{ 0, {} }, false};
//    }
//
//
//    static NMR Quiescence(const BoardPtr& board, const State state, int depth, float alpha, float beta) {
//        float stand_pat = subjectiveEval(evaluation::position_evaluate(board), state);
//
//        if (stand_pat >= beta) {
//            nodesSearched++;
//            return { beta, {} };
//        }
//        if (alpha < stand_pat) {
//            alpha = stand_pat;
//        }
//
//        PDptr pd = state.whiteToMove ? CheckLogicHandler::reload<true>(board) : CheckLogicHandler::reload<false>(board);
//
//        moves.at(depth).clear();
//        currentDepth = depth;
//
//        generate<EngineMC>(board, state, pd);
//
//        if(moves.at(depth).empty()) {
//            nodesSearched++;
//            return {alpha, {}};
//        }
//
//        std::vector<Move> bestLine;
//
//        /// Iterate through all moves
//        for(auto& move_pair: moves.at(depth)) {
//            auto [info, move] = move_pair;
//            auto [nextBoardPtr, nextState] = forkBoard(board, state, move);
//            auto [eval, line] = Quiescence(nextBoardPtr, nextState, depth + 1,  -beta,  -alpha);
//            eval = -eval;
//
//            if (eval >= beta) {
//                return { beta, {} };
//            }
//            if (eval > alpha) {
//                alpha = eval;
//                line.push_back(move);
//                bestLine = line;
//            }
//        }
//        return { alpha, bestLine };
//    }
//
//    template<bool topLevel>
//    static NMR negamax(const BoardPtr& board, const State state, int depth, float alpha, float beta) {
//        /// Lookup position in table
//        size_t boardHash;
//        if constexpr (!topLevel) {
//            boardHash = Zobrist::hash(board, state);
//            std::pair<NMR, bool> res = retrieveFromLookupTable(boardHash, alpha, beta, depth);
//            if (res.second) {
//                return res.first;
//            }
//        }
//
//    /// Recursion Base Case: Max Depth reached -> return heuristic position eval
//        if (depth > maxDepth) {
//            return Quiescence(board, state, 1, alpha, beta);
//        }
//
////        bool expandMove = depth < boundaryDepth;
//        // Either expand Move or set heuristic val
//        // expand if capture or check
////        if(!expandMove) {
////            BB enemyPieces = state.whiteToMove ? board.enemyPieces<true>() : board.enemyPieces<false>();
////            if(hasBitAt(enemyPieces, move.toIndex)) {
////                expandMove = true;
////                std::cout << "Extending Depth for " << Utils::moveNameNotation(move) << std::endl;
////            }
////        }
//
//        PDptr pd = state.whiteToMove ? CheckLogicHandler::reload<true>(board) : CheckLogicHandler::reload<false>(board);
//
//        moves.at(depth).clear();
//        currentDepth = depth;
//
////        if constexpr (forcingMovesOnly) {
////            generate<EngineMC, true>(board, state, pd);
////
////            if(moves.at(depth).empty()) {
////                nodesSearched++;
////                float heuristic_val = evaluation::position_evaluate(board);
////                float eval = subjectiveEval(heuristic_val, state);
////                saveToLookupTable(eval, boardHash, alpha, beta, depth);
////                return {eval, {}};
////            }
////        } else {
//
//        generate<EngineMC>(board, state, pd);
//
//        bool checkmated = pd->inCheck() && moves.at(depth).empty();
//        if (checkmated) {
//            nodesSearched++;
//            return {-INF, {}};
//        }
//
//        if (moves.at(depth).empty()) {
//            // Stalemate!
//            nodesSearched++;
//            return {0, {}};
//        }
//
////        }
//
////        std::sort(moves.at(depth).begin(), moves.at(depth).end(), sortMovePairs);
//
////        if constexpr (topLevel) {
////            for(auto& x: moves.at(depth)) {
////                std::cout << "(" << x.first << ", " << Utils::moveNameNotation(x.second) << ") ";
////            }
////            std::cout << std::endl;
////        }
//
//
//        float currentEval = -1000000;
//        std::vector<Move> bestLine;
//
//        /// Iterate through all moves
//        for(auto& move_pair: moves.at(depth)) {
//            auto [info, move] = move_pair;
//
////            for (int i = 0; i < depth; i++)
////                std::cout << " ";
////            std::cout << depth << " : " << Utils::moveNameNotation(move) << std::endl;
//
//
////            if(expandMove) {
//            auto [nextBoardPtr, nextState] = forkBoard(board, state, move);
//            auto [eval, line] = negamax<false>(nextBoardPtr, nextState, depth + 1,  -beta,  -alpha);
//            eval = -eval;
////            } else {
////                nodesSearched++;
////                float heuristic_val = evaluation::position_evaluate(board);
////                nodesSearched++;
////                eval = subjectiveEval(heuristic_val, state);
//////                return {subjectiveEval(heuristic_val, state), {}};
////            }
//
////            if constexpr (topLevel) {
////                std::cout << "EVAL: " << subjectiveEval(eval, state) << std::endl;
////                Utils::printMove(move);
////            }
//
//            if (eval > currentEval) {
//                currentEval = eval;
//                line.push_back(move);
//                bestLine = line;
//
////                if constexpr (topLevel) {
////                    bestMoves.clear();
////                    bestMoves.push_back(move);
////                }
////            } else {
////                if constexpr (topLevel) {
////                    float margin = bestMoves.size() >= MARGIN_THRESHOLD ? 0 : BEST_MOVE_MARGIN;
////                    if (eval >= currentEval - margin && bestMoves.size() < MAX_NUMBER_BEST_MOVES) {
////                        bestMoves.push_back(move);
////                    }
////                }
//            }
//
////            if constexpr (!topLevel) {
//                if (currentEval > alpha) {
//                    alpha = currentEval;
//                }
//                if (alpha >= beta) break;
////            }
//        }
//
//        /// Save to lookup table
//        if constexpr (!topLevel) {
//            saveToLookupTable(currentEval, boardHash, alpha, beta, depth);
//        }
//
//        return {currentEval, bestLine};
//    }
//
//    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
//    static void registerMove(const BoardPtr &board, BB from, BB to) {
//        moves[currentDepth].emplace_back(
//            evaluation::move_heuristic<state, piece, flags>(board, from, to),
//            Move(from, to, piece, flags)
//        );
//    }
//
////    template<State nextState>
////    static void next(Board& nextBoard) {}
//
//    friend class MoveGenerator<EngineMC>;
//};
//
//std::array<std::vector<std::pair<float, Move>>, 32> EngineMC::moves{};
//int EngineMC::currentDepth{0};
//int EngineMC::boundaryDepth{0};
//int EngineMC::maxDepth{0};
//float EngineMC::evaluation{0};
//BB EngineMC::nodesSearched{0};
//BB EngineMC::lookups{0};
//std::vector<Move> EngineMC::bestMoves{};
//std::unordered_map<BB, TTEntry> EngineMC::lookup_table;
//
//
//
//
////template<int d>
////Move EngineMC<d>::bestMove{};
//
////template<bool saveList, bool print>
////unsigned long long EngineMC<saveList, print>::totalNodes{0};
////template<bool saveList, bool print>
////std::vector<Board> EngineMC<saveList, print>::positions{};
//
//#endif //DORY_ENGINE_H
