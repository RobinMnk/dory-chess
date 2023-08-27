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

class EngineMC {
public:
    template<State state>
    static double generateGameTree(const Board& board, int depth) {
        currentDepth = 0;
        bestMoves.clear();
        return negamax<true>(board, toCode<state>(), depth, -INF, INF);
    }

    static std::vector<Move> bestMoves;
private:
    static std::array<std::array<Move, 256>, 16> moves;
    static std::array<int, 16> numMoves;
    static int currentDepth;

    template<bool topLevel>
    static double negamax(const Board& board, uint8_t state_code, int depth, double alpha, double beta) {
        if (depth <= 0) {
            State state = toState(state_code);
            double heuristic_val = evaluation::hard_evaluate(board);
            return state.whiteToMove ? heuristic_val : -heuristic_val;
        }

        double currentEval = -9999;

        currentDepth++;
        numMoves[currentDepth] = 0;

//        Utils::print_board(board);

        bool checkmated = generate<EngineMC>(board, state_code);

        if(checkmated) {
//            std::cout << "Checkmate found!" << std::endl;
            currentDepth--;
            return -INF;
        }

        // Potentially order moves

        int ctr = 0;
        for(auto& move: moves[currentDepth]) {
            if (ctr++ >= numMoves[currentDepth])
                break;

            auto [nextBoard, nextStateCode] = make_move(board, state_code, move);

            double eval = - negamax<false>(nextBoard, nextStateCode, depth - 1, - beta, - alpha);


            if constexpr (topLevel) {
                if (eval == currentEval) {
                    bestMoves.push_back(move);
                }
            }

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

        currentDepth--;
        return currentEval;
    }

    template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {
        int ix = numMoves[currentDepth];
        moves[currentDepth][ix].from = from;
        moves[currentDepth][ix].to = to;
        moves[currentDepth][ix].piece = piece;
        moves[currentDepth][ix].flags = flags;
        numMoves[currentDepth]++;
    }

    template<State nextState>
    static void next(Board& nextBoard) {

    }

    friend class MoveGenerator<EngineMC>;
};

std::array<std::array<Move, 256>, 16> EngineMC::moves{};
std::array<int, 16> EngineMC::numMoves{0};
int EngineMC::currentDepth{0};
std::vector<Move> EngineMC::bestMoves{};


//template<int d>
//Move EngineMC<d>::bestMove{};

//template<bool saveList, bool print>
//unsigned long long EngineMC<saveList, print>::totalNodes{0};
//template<bool saveList, bool print>
//std::vector<Board> EngineMC<saveList, print>::positions{};

#endif //DORY_ENGINE_MOVE_COLLECTOR_H
