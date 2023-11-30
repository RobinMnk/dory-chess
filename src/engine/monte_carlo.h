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
    static Utils::Random random;
public:

    static std::string simulateGame(const Board& startBoard, State startState, int depth) {

        Move nextMove;
        Board currentBoard = startBoard;
        State currentState = startState;

        EngineMC::reset();

        int ply = 0;
        std::stringstream fen{};
        while(true) {
            ply++;

//            Utils::print_board(currentBoard);
//            std::cout << (currentState.whiteToMove ? "White" : "Black") << " to move" << std::endl;
            auto [eval, line] = EngineMC::searchDepth(currentBoard, currentState, depth);

            if (ply > 150 || EngineMC::topLevelLegalMoves().empty()) {
                std::cout << "END of Game!" << std::endl;
                break;
            }

//            printf("%zu\n", EngineMC::bestMoves().size());
//            printf("%zu\n", EngineMC::topLevelLegalMoves().size());

//            nextMove = line.back();

            if (random.bernoulli(USE_ENGINE_BEST_MOVES_PROBABILITY)) {
                nextMove = random.randomElementOf(EngineMC::bestMoves());
            } else {
                std::cout << "Picking legal move at random" << std::endl;
                nextMove = random.randomElementOf(EngineMC::topLevelLegalMoves()).second;
            }

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

Utils::Random MonteCarlo::random{};


#endif //DORY_MONTE_CARLO_H
