//
// Created by robin on 28.08.2023.
//

#ifndef DORY_MONTE_CARLO_H
#define DORY_MONTE_CARLO_H

#include "engine_move_collector.h"
#include "../random.h"

double USE_ENGINE_BEST_MOVES_PROBABILITY = 1;


class MonteCarlo {
    Utils::Random random{};
public:

    std::basic_string<char, std::char_traits<char>, std::allocator<char>> simulateGame(const Board& startBoard, State startState) {

        Move nextMove;
        Board currentBoard = startBoard;
        State currentState = startState;

        int ply = 0;
        std::stringstream fen{};
        while(true) {
            ply++;

//            Utils::print_board(currentBoard);
//            std::cout << (currentState.whiteToMove ? "White" : "Black") << " to move" << std::endl;
            double eval = EngineMC::beginEvaluation(currentBoard, currentState, 2);

            if (EngineMC::bestMoves.empty() || ply > 50) {
                std::cout << "END of Game!" << std::endl;
                break;
            }

            if (random.bernoulli(USE_ENGINE_BEST_MOVES_PROBABILITY)) {
                nextMove = random.randomElementOf(EngineMC::bestMoves);
            } else {
                std::cout << "We should not be here!:  " << std::endl;
                nextMove = random.randomElementOf(EngineMC::topLevelLegalMoves()).second;
            }

            std::cout << "Eval:  " << eval << std::endl;
            Utils::printMove(nextMove);

            auto [nextBoard, nextState] = make_move(currentBoard, currentState, nextMove);

            currentBoard = nextBoard;
            currentState = nextState;

            if (ply % 2 == 1) {
                fen << (1 + (ply >> 1)) << ". ";
            }
            fen << Utils::moveNameNotation(nextMove) << " ";
        }

//        delete currentBoard;
//        delete currentState;

        fen << "\n";
        return fen.str();
    }
};


#endif //DORY_MONTE_CARLO_H
