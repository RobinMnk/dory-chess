//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_MOVE_COLLECTOR_H
#define DORY_ENGINE_MOVE_COLLECTOR_H

#include <iostream>
#include <stack>
#include <vector>
#include <memory>
#include <thread>
#include <unordered_map>
#include "../movegen.h"
#include "evaluation.h"
#include "../utils.h"
#include "../zobrist.h"

float INF = 99999;

int DEPTH_MARGIN_SIZE = 2;
float BEST_MOVE_MARGIN = 0.05;
size_t MAX_NUMBER_BEST_MOVES = 6;
size_t MARGIN_THRESHOLD = 3;

using NMR = std::pair<float, std::vector<Move>>;

class GameTree {
    struct TreeNode {
        int offset{};
        float eval{};
        Move move; // TODO: Idea -> move to external vector and save only index here!
    };

    using TPT = std::shared_ptr<TreeNode>;

    std::vector<std::vector<TPT>> tree{};
    TPT root;
    Board* startBoard;
    State startState;

public:
    GameTree (const Board& board, State& state) : startBoard{new Board(board)}, startState(state) {
        root = std::make_unique<TreeNode>();
    }

    void expand() {
        TPT parent = root;

        int index = 0, level = 0;
        Board* currentBoard = startBoard;
        State currentState = startState;

        for(TPT node: tree.at(level)) {
            if(index == root->offset) {
                // Expand this node

                continue;
            }

            index++;
        }

    }
};


float subjectiveEval(float eval, State state) {
    return state.whiteToMove ? eval : -eval;
}

bool sortMovePairs(const std::pair<float, Move> &a, const std::pair<float, Move> &b) {
    return a.first < b.first;
}

const uint8_t TTFlagExact = 0, TTFlagLowerBound = 1, TTFlagUpperBound = 2;

struct TTEntry {
    float value;
    uint8_t depth, flag;
};

class EngineMC {
public:
//    template<State state>
//    static void start(const Board& board, int depth) {
//        std::jthread thr{[&board, &depth]{
//            beginEvaluation<state>(board, depth);
//        }};
//    }

    static NMR beginEvaluation(const Board& board, State state, int md) {
        boundaryDepth = md;
        maxDepth = md + DEPTH_MARGIN_SIZE;
        bestMoves.clear();
        lookup_table.clear();
        nodesSearched = 0;
        auto [ev, line] = negamax<true>(board, state, 1, -INF, INF);
        bestMoves.clear();
        bestMoves.push_back(line.back());
        return {subjectiveEval(ev, state), line};
    }

    static float evaluation;
    static std::vector<Move> bestMoves;
    static BB nodesSearched, lookups;

    static std::vector<std::pair<float, Move>> topLevelLegalMoves() {
        return moves[0];
    }
private:
    static std::array<std::vector<std::pair<float, Move>>, 26> moves;
    static int maxDepth, boundaryDepth, currentDepth;
    static std::unordered_map<BB, TTEntry> lookup_table;


    template<bool topLevel>
    static NMR negamax(const Board& board, State state, int depth, float alpha, float beta) {
        float origAlpha = alpha;

        size_t boardHash;
        if constexpr (!topLevel) {
            boardHash = Zobrist::hash(board, state);
            auto res = lookup_table.find(boardHash);

            if(res != lookup_table.end()) {
                TTEntry entry = res->second;
                if (entry.depth <= depth) {
                    lookups++;
                    if (entry.flag == TTFlagExact) {
                        return {entry.value, {}};
                    } else if (entry.flag == TTFlagLowerBound) {
                        if (entry.value > alpha) alpha = entry.value;
                    } else if (entry.flag == TTFlagUpperBound) {
                        if (entry.value < beta) beta = entry.value;
                    }

                    if (alpha >= beta) return {entry.value, {}};
                }
            }
        }

        if (depth >= maxDepth) {
            float heuristic_val = evaluation::position_evaluate(board);
            nodesSearched++;
            return {subjectiveEval(heuristic_val, state), {}};
        }

        float currentEval = -1000000;
        std::vector<Move> bestLine;

        moves[depth].clear();
        currentDepth = depth;

        bool checkmated = generate<EngineMC>(board, state);

        if (checkmated) {
            nodesSearched++;
            return {-INF, {}};
        }

//        std::cout << depth << std::endl;

        if (moves[depth].empty()) {
            // Stalemate!
            nodesSearched++;
            return {0, {}};
        }

        std::sort(moves[depth].begin(), moves[depth].end(), sortMovePairs);

        for(auto& move_pair: moves[depth]) {
            auto [info, move] = move_pair;

//            std::cout << depth << " : " << Utils::moveNameNotation(move) << std::endl;

            float eval{};
            bool expandMove = depth <= boundaryDepth;
            // Either expand Move or set heuristic val
            // expand if capture or check
            if(!expandMove) {
                BB enemyPieces = state.whiteToMove ? board.enemyPieces<true>() : board.enemyPieces<false>();
                if(hasBitAt(enemyPieces, move.toIndex)) {
                    expandMove = true;
//                    std::cout << "Extending Depth for " << Utils::moveNameNotation(move) << std::endl;
                }
            }

            std::vector<Move> line;
            if(expandMove) {
                auto [nextBoard, nextState] = make_move(board, state, move);
                auto [ev, ln] = negamax<false>(nextBoard, nextState, depth + 1,  -beta,  -alpha);
                eval = -ev;
                line = ln;
            } else {
                nodesSearched++;
                float heuristic_val = evaluation::position_evaluate(board);
                return {subjectiveEval(heuristic_val, state), {}};
            }

//            if constexpr (topLevel) {
//                std::cout << "EVAL: " << subjectiveEval(eval, state) << std::endl;
//                Utils::printMove(move);
//            }

            if (eval > currentEval) {
                currentEval = eval;
                line.push_back(move);
                bestLine = line;

//                if constexpr (topLevel) {
//                    bestMoves.clear();
//                    bestMoves.push_back(move);
//                }
//            } else {
//                if constexpr (topLevel) {
//                    float margin = bestMoves.size() >= MARGIN_THRESHOLD ? 0 : BEST_MOVE_MARGIN;
//                    if (eval >= currentEval - margin && bestMoves.size() < MAX_NUMBER_BEST_MOVES) {
//                        bestMoves.push_back(move);
//                    }
//                }
            }

//            if constexpr (!topLevel) {
                if (currentEval > alpha) {
                    alpha = currentEval;
                }
                if (alpha >= beta) break;
//            }
        }

        if constexpr (!topLevel) {
            uint8_t flag{};
            if (currentEval <= origAlpha)
                flag = TTFlagUpperBound;
            else if (currentEval >= beta)
                flag = TTFlagLowerBound;
            else flag = TTFlagExact;

            TTEntry entry{currentEval, static_cast<uint8_t>(depth), flag};
            lookup_table.insert({boardHash, entry});
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

    template<State nextState>
    static void next(Board& nextBoard) {}

    friend class MoveGenerator<EngineMC>;
};

std::array<std::vector<std::pair<float, Move>>, 26> EngineMC::moves{};
int EngineMC::currentDepth{0};
int EngineMC::boundaryDepth{0};
int EngineMC::maxDepth{0};
float EngineMC::evaluation{0};
BB EngineMC::nodesSearched{0};
BB EngineMC::lookups{0};
std::vector<Move> EngineMC::bestMoves{};
std::unordered_map<BB, TTEntry> EngineMC::lookup_table;




//template<int d>
//Move EngineMC<d>::bestMove{};

//template<bool saveList, bool print>
//unsigned long long EngineMC<saveList, print>::totalNodes{0};
//template<bool saveList, bool print>
//std::vector<Board> EngineMC<saveList, print>::positions{};

#endif //DORY_ENGINE_MOVE_COLLECTOR_H
