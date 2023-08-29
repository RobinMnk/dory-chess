//
// Created by robin on 28.08.2023.
//

#ifndef DORY_MONTE_CARLO_H
#define DORY_MONTE_CARLO_H

#include <random>
#include "engine_move_collector.h"

using RNG = std::uniform_int_distribution<std::mt19937::result_type>;

double USE_ENGINE_BEST_MOVES_PROBABILITY = 1;


class MonteCarlo {

    class MCRandom {
        std::mt19937 rng;

    public:
        MCRandom() {
            std::random_device dev;
            rng = std::mt19937(dev());
        }

        size_t randomNumberInRange(int lo, int hi) {
            RNG dist6(lo, hi); // distribution in range [1, 6]
            return dist6(rng);
        }

        template<typename T>
        T randomElementOf(std::vector<T>&& list) {
            size_t index = randomNumberInRange(0, list.size()-1);
            return list.at(index);
        }

        template<typename T>
        T randomElementOf(std::vector<T>& list) {
            size_t index = randomNumberInRange(0, list.size()-1);
            return list.at(index);
        }

        bool bernoulli(double p) {
            size_t num = randomNumberInRange(0, INT32_MAX);
            double res = static_cast<double>(num) / static_cast<double>(INT32_MAX);
            return res <= p;
        }

    };


    MCRandom random{};
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
            double eval = EngineMC::beginEvaluation(currentBoard, currentState, 5);

            if (EngineMC::bestMoves.empty() || ply > 100) {
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
