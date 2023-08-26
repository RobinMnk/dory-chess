//
// Created by robin on 24.08.2023.
//

#ifndef DORY_ENGINE_MOVE_COLLECTOR_H
#define DORY_ENGINE_MOVE_COLLECTOR_H

#include <iostream>
#include <stack>
#include <vector>
#include "../movegen.h"
#include "evaluation.h"
#include "../utils.h"

class EngineMC {
public:
    template<State state, int maxDepth>
    static void generateGameTree(const Board& board) {
        add_dummy<state.whiteToMove>();
        build<state, maxDepth>(board);
        evaluation = level_best.top();
        if constexpr (state.whiteToMove)
            evaluation *= -1;
        level_best.pop();
    }

    static std::vector<Move> line;
    static double evaluation;
private:
    static std::stack<double> level_best;
    static Move currentMove;

    template<bool whiteToMove>
    static constexpr void add_dummy() {
        level_best.push(-9999);
    }

    template<State state, int depth>
    static void build(const Board& board) {
        if constexpr (depth > 0) {
            add_dummy<state.whiteToMove>();

            bool checkmated = MoveGenerator<EngineMC>::template generate<state, depth>(board);


            double best_from_below;
            if (checkmated) {
                best_from_below = 999;
                std::cout << "Checkmate!  " << state.whiteToMove << std::endl;
                Utils::print_board(board);
            } else {
                best_from_below = - level_best.top();
                level_best.pop();
            }

//            line.pop_back();
            double current = level_best.top();
            if (best_from_below > current) {
                level_best.pop();
                level_best.push(best_from_below);

//                line.emplace_back(best_move_below);
            }

        } else {
            // Leaf node reached
            double heuristic_val = evaluation::hard_evaluate<state>(board);

            if constexpr (state.whiteToMove) {
                heuristic_val *= -1;
            }

//            std::cout << " ------------------  " << std::endl;
//            std::cout << "Evaluating: " << (state.whiteToMove ? "white" : "black" ) << " to move" << std::endl;
//            Utils::print_board(board);
//            std::cout << "EVAL:   " << heuristic_val << std::endl;
//            Utils::printMoveList(line);
            if (heuristic_val > level_best.top()) {
                level_best.pop();
                level_best.push(heuristic_val);

//                line.pop_back();
//                line.emplace_back(currentMove);
//                std::cout << "UPDATING EVAL!" << std::endl;

//                bestMove = currentMove;
//                std::cout << "Updating eval: " << eval << std::endl;
//            } else {
//                line.pop_back();
            }
//            std::cout << std::endl;
        }
    }

    template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
    static void registerMove(const Board &board, BB from, BB to) {
        currentMove = Move{from, to, piece, flags};
//        line.emplace_back(currentMove);
//        std::cout << "Receiving move " << std::endl;
//        Utils::printMove(currentMove);
    }

    template<State nextState, int depth>
    static void next(Board& nextBoard) {
        build<nextState, depth-1>(nextBoard);
    }

    friend class MoveGenerator<EngineMC>;
};

std::stack<double> EngineMC::level_best{};
double EngineMC::evaluation{0.};
Move EngineMC::currentMove{};
std::vector<Move> EngineMC::line{};


//template<int d>
//Move EngineMC<d>::bestMove{};

//template<bool saveList, bool print>
//unsigned long long EngineMC<saveList, print>::totalNodes{0};
//template<bool saveList, bool print>
//std::vector<Board> EngineMC<saveList, print>::positions{};

#endif //DORY_ENGINE_MOVE_COLLECTOR_H
