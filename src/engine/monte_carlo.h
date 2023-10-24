//
// Created by robin on 28.08.2023.
//

#ifndef DORY_MONTE_CARLO_H
#define DORY_MONTE_CARLO_H

#include "engine.h"
#include "../random.h"

double USE_ENGINE_BEST_MOVES_PROBABILITY = 1;



//class GameTree {
//    struct TreeNode {
//        uint8_t wins{0}, total{0};
//        float eval{};
//        Move move;
//    };
//
//    using TPT = std::shared_ptr<TreeNode>;
//
//    std::vector<std::vector<TPT>> tree{};
//    TPT root;
//    Board* startBoard;
//    State startState;
//
//public:
//    GameTree (const Board& board, State& state) : startBoard{new Board(board)}, startState(state) {
//        root = std::make_unique<TreeNode>();
//    }
//
//    void expand() {
//        TPT parent = root;
//
//        int index = 0, level = 0;
//        Board* currentBoard = startBoard;
//        State currentState = startState;
//
//        for(TPT node: tree.at(level)) {
////            if(index == root->offset) {
////                // Expand this node
////
////                continue;
////            }
//
//            index++;
//        }
//
//    }
//};


class MonteCarlo {
    Utils::Random random{};
public:

    static std::basic_string<char, std::char_traits<char>, std::allocator<char>> simulateGame(const Board& startBoard, State startState) {

        Move nextMove;
        Board currentBoard = startBoard;
        State currentState = startState;

        int ply = 0;
        std::stringstream fen{};
        while(true) {
            ply++;

//            Utils::print_board(currentBoard);
//            std::cout << (currentState.whiteToMove ? "White" : "Black") << " to move" << std::endl;
            auto [eval, line] = EngineMC::beginEvaluation(currentBoard, currentState, 6);

            if (ply > 50) {
                std::cout << "END of Game!" << std::endl;
                break;
            }

            nextMove = line.back();

//            if (random.bernoulli(USE_ENGINE_BEST_MOVES_PROBABILITY)) {
//                nextMove = random.randomElementOf(EngineMC::bestMoves);
//            } else {
//                std::cout << "We should not be here!:  " << std::endl;
//                nextMove = random.randomElementOf(EngineMC::topLevelLegalMoves()).second;
//            }

//            std::cout << "Eval:  " << eval << std::endl;
            Utils::printMove(nextMove);

//            auto [nextBoard, nextState] = forkBoard(*currentBoard, currentState, nextMove);

//            currentBoard = nextBoard;
//            currentState = nextState;

            currentBoard.makeMove(currentState, nextMove);
            currentState.update(nextMove.flags);

            if (ply % 2 == 1) {
                fen << (1 + (ply >> 1)) << ". ";
            }
            fen << Utils::moveNameNotation(nextMove) << " ";
        }

        fen << "\n";
        return fen.str();
    }
};


#endif //DORY_MONTE_CARLO_H
