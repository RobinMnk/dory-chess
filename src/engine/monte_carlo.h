//
// Created by robin on 28.08.2023.
//

#ifndef DORY_MONTE_CARLO_H
#define DORY_MONTE_CARLO_H

#include "engine.h"
#include "../random.h"

double USE_ENGINE_BEST_MOVES_PROBABILITY = 0.9;


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
//    Board startBoard;
//    State startState;
//
//public:
//    GameTree (const Board& board, State& state) : startBoard{board}, startState(state) {
//        root = std::make_unique<TreeNode>();
//    }
//
//    void expand() {
//        TPT parent = root;
//
//        int index = 0, level = 0;
//        Board currentBoard = startBoard;
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

    static void runSimulations(const Board& board, State state, int depth, int numSimulations) {
        int wins{0}, losses{0};
        for(int i = 1; i <= numSimulations; i++) {
            int res = simulateGame(board, state, depth);
            if(res == 1) wins++;
            else if (res == -1) losses++;

        }
        std::cout << "\n" << wins << " wins, " << losses << " losses, " << numSimulations << " games" << std::endl;
    }

    static int simulateGame(const Board& startBoard, State startState, int depth) {
        Board currentBoard = startBoard;
        State currentState = startState;
        bool white = startState.whiteToMove;

        EngineMC::reset();

        int action_counter = 0;
        Move nextMove;
//        std::stringstream fen{};


        // After 200 moves of simulation a game is considered a draw
        for(int ply{0}; ply < 400; ply++) {

//            Utils::print_board(currentBoard);
//            std::cout << (currentState.whiteToMove ? "White" : "Black") << " to move" << std::endl;
            auto [eval, line] = EngineMC::iterativeDeepening(currentBoard, currentState, depth);

            if (line.empty()) {
                // Game over
                if(eval > (INF - 50)) {
                    // White wins by Checkmate
                    std::cout << "Checkmate - White wins!  (" << (ply / 2) << " moves)" << std::endl;
                    return white ? 1 : -1;
                } else if(eval < -(INF - 50)) {
                    // Black wins by Checkmate
                    std::cout << "Checkmate - Black wins!  (" << (ply / 2) << " moves)" << std::endl;
                    return white ? -1 : 1;
                }
                std::cout << "Draw - Threefold repetition!  (" << (ply / 2) << " moves)" << std::endl;
                return 0;
            }


//            printf("%zu\n", EngineMC::bestMoves().size());
//            printf("%zu\n", EngineMC::topLevelLegalMoves().size());

//            nextMove = line.back();

            if (random.bernoulli(USE_ENGINE_BEST_MOVES_PROBABILITY)) {
                nextMove = random.randomElementOf(EngineMC::bestMoves());
            } else {
//                std::cout << "Picking legal move at random" << std::endl;
                nextMove = random.randomElementOf(EngineMC::topLevelLegalMoves()).second;
            }

//            std::cout << "Eval:  " << eval << std::endl;
//            Utils::printMove(nextMove);

            // Count moves since last pawn move or capture
            if(nextMove.piece == Piece::Pawn || currentBoard.isCapture(currentState, nextMove)) {
                action_counter = 0;
                EngineMC::repTable.reset();
            } else if(action_counter++ >= 100) {
                std::cout << "Draw - 50 move rule!  (" << (ply / 2) << " moves)" << std::endl;
                return 0;
            }


            EngineMC::repTable.insert(Zobrist::hash(currentBoard, currentState));

            currentBoard.makeMove(currentState, nextMove);
            currentState.update(nextMove.flags);

//            if (ply % 2 == 1) {
//                fen << (1 + (ply >> 1)) << ". ";
//            }
//            fen << Utils::moveNameNotation(nextMove) << " ";
        }

        std::cout << "Game aborted" << std::endl;
        return 0;

//        fen << "\n";
//        return fen.str();
//        throw std::runtime_error("Game simulation aborted unexpectedly");
    }
};

Utils::Random MonteCarlo::random{};


#endif //DORY_MONTE_CARLO_H
