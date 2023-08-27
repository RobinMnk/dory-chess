//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_MOVE_COLLECTOR_H
#define DORY_ENGINE_MOVE_COLLECTOR_H

#include <iostream>
#include <stack>
#include <vector>
#include <memory>
#include "../movegen.h"
#include "evaluation.h"
#include "../utils.h"

double INF = 99999;

class IterativeDeepening {
public:
    void run() {

    }
};

struct MoveInfo {
    double heuristic_value;
    bool depthExtension;
};

template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
static bool depthExtension(const Board &board, BB from, BB to) {
    if ((to & board.enemyPieces<state.whiteToMove>()) != 0) {
        // keep searching if was capture
        return false;
    }
    return false;
}

bool sortMovePairs(const std::pair<MoveInfo, Move> &a, const std::pair<MoveInfo, Move> &b) {
    return a.first.heuristic_value > b.first.heuristic_value;
}


class EngineMC {
public:
    template<State state>
    static double generateGameTree(const Board& board, int depth) {
        maxDepth = depth;
        bestMoves.clear();
        double eval = negamax<true>(board, toCode<state>(), 0, -INF, INF);
        if constexpr (state.whiteToMove) {
            return eval;
        }
        return -eval;
    }

    static std::vector<Move> bestMoves;
private:
    static std::array<std::vector<std::pair<MoveInfo, Move>>, 16> moves;
    static std::array<int, 16> numMoves;
    static int maxDepth, currentDepth;

    template<bool topLevel>
    static double negamax(const Board& board, uint8_t state_code, int depth, double alpha, double beta, bool extension = false) {
        State state = toState(state_code);
        if ((depth >= maxDepth && !extension) || depth > maxDepth + 2) {
            double heuristic_val = evaluation::position_evaluate(board);
            return state.whiteToMove ? heuristic_val : -heuristic_val;
        }

        double currentEval = -1000000;

        moves[depth].clear();

        currentDepth = depth;
        bool checkmated = generate<EngineMC>(board, state_code);
        std::sort(moves[depth].begin(), moves[depth].end(), sortMovePairs);

        if(checkmated) {
            return -INF;
        }

        for(auto& move_pair: moves[depth]) {
            auto [info, move] = move_pair;
            auto [nextBoard, nextStateCode] = make_move(board, state_code, move);

            double eval = - negamax<false>(nextBoard, nextStateCode, depth + 1, - beta, - alpha, info.depthExtension);

//            if constexpr (topLevel) {
//                std::cout << eval << std::endl;
//                if (eval == currentEval) {
//                    bestMoves.push_back(move);
//                }
//            }

            if (eval > currentEval) {
                currentEval = eval;

                if constexpr (topLevel) {
                    bestMoves.clear();
                    bestMoves.push_back(move);
                }
            }

            if (eval > alpha) {
                alpha = eval;
            }

            if (alpha >= beta) break;
        }

        return currentEval;
    }

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {
        MoveInfo info{evaluation::move_heuristic<state, piece, flags>(board, from, to),
                      depthExtension<state, piece, flags>(board, from, to)};

        moves[currentDepth].emplace_back(
            info,
            Move(from, to, piece, flags)
        );
    }

    template<State nextState>
    static void next(Board& nextBoard) {}

    friend class MoveGenerator<EngineMC>;
};

std::array<std::vector<std::pair<MoveInfo, Move>>, 16> EngineMC::moves{};
std::array<int, 16> EngineMC::numMoves{0};
int EngineMC::currentDepth{0};
int EngineMC::maxDepth{0};
std::vector<Move> EngineMC::bestMoves{};



//template<int d>
//Move EngineMC<d>::bestMove{};

//template<bool saveList, bool print>
//unsigned long long EngineMC<saveList, print>::totalNodes{0};
//template<bool saveList, bool print>
//std::vector<Board> EngineMC<saveList, print>::positions{};

#endif //DORY_ENGINE_MOVE_COLLECTOR_H
