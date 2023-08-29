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
#include "../movegen.h"
#include "evaluation.h"
#include "../utils.h"

double INF = 99999;

double BEST_MOVE_MARGIN = 0.05;
size_t MAX_NUMBER_BEST_MOVES = 6;
size_t MARGIN_THRESHOLD = 3;

class GameTree {
    struct TreeNode {
        int offset{};
        double eval{};
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


bool sortMovePairs(const std::pair<double, Move> &a, const std::pair<double, Move> &b) {
    return a.first > b.first;
}

class EngineMC {
public:
//    template<State state>
//    static void start(const Board& board, int depth) {
//        std::jthread thr{[&board, &depth]{
//            beginEvaluation<state>(board, depth);
//        }};
//    }

    static double beginEvaluation(const Board& board, State state, int md) {
        maxDepth = md;
        bestMoves.clear();
        nodesSearched = 0;
        evaluation = negamax<true>(board, state, 0, -INF, INF);
        if (!state.whiteToMove) evaluation = - evaluation;
        return evaluation;
    }

    static double evaluation;
    static std::vector<Move> bestMoves;
    static BB nodesSearched;

    static std::vector<std::pair<double, Move>> topLevelLegalMoves() {
        return moves[0];
    }
private:
    static std::array<std::vector<std::pair<double, Move>>, 16> moves;
    static int maxDepth, currentDepth;

    template<bool topLevel>
    static double negamax(const Board& board, State state, int depth, double alpha, double beta) {
        if (depth >= maxDepth) {
            double heuristic_val = evaluation::position_evaluate(board);
            nodesSearched++;
            return state.whiteToMove ? heuristic_val : -heuristic_val;
        }

        double currentEval = -1000000;

        moves[depth].clear();
        currentDepth = depth;

        bool checkmated = generate<EngineMC>(board, state);

        if (checkmated) {
            nodesSearched++;
            return -INF;
        }

        if (moves[depth].empty()) {
            // Stalemate!
            nodesSearched++;
            return 0;
        }

        std::sort(moves[depth].begin(), moves[depth].end(), sortMovePairs);

        for(auto& move_pair: moves[depth]) {
            auto [info, move] = move_pair;
            auto [nextBoard, nextState] = make_move(board, state, move);

            double eval = - negamax<false>(nextBoard, nextState, depth + 1,  -beta,  -alpha);

            if (eval > currentEval) {
                currentEval = eval;

                if constexpr (topLevel) {
                    bestMoves.clear();
                    bestMoves.push_back(move);
                }
            } else {
                if constexpr (topLevel) {
                    double margin = bestMoves.size() >= MARGIN_THRESHOLD ? 0 : BEST_MOVE_MARGIN;
                    if (eval >= currentEval - margin && bestMoves.size() < MAX_NUMBER_BEST_MOVES) {
                        bestMoves.push_back(move);
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

        return currentEval;
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

std::array<std::vector<std::pair<double, Move>>, 16> EngineMC::moves{};
int EngineMC::currentDepth{0};
int EngineMC::maxDepth{0};
double EngineMC::evaluation{0};
BB EngineMC::nodesSearched{0};
std::vector<Move> EngineMC::bestMoves{};



//template<int d>
//Move EngineMC<d>::bestMove{};

//template<bool saveList, bool print>
//unsigned long long EngineMC<saveList, print>::totalNodes{0};
//template<bool saveList, bool print>
//std::vector<Board> EngineMC<saveList, print>::positions{};

#endif //DORY_ENGINE_MOVE_COLLECTOR_H
