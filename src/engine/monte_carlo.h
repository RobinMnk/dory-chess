//
// Created by Robin on 28.08.2023.
//

#ifndef DORY_MONTE_CARLO_H
#define DORY_MONTE_CARLO_H

#include "engine.h"
#include "../utils/random.h"

double USE_ENGINE_BEST_MOVES_PROBABILITY = 0.95;
double USE_RANDOM_MOVE_IN_ROLLOUT = 0.5;

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
        for(int ply{0}; ply < 600; ply++) {

//            Utils::print_board(currentBoard);
//            std::cout << (currentState.whiteToMove ? "White" : "Black") << " to move" << std::endl;
            auto [eval, line] = EngineMC::iterativeDeepening(currentBoard, currentState, depth);

            if (line.empty()) {
                // Game over
                if(eval > (INF - 50)) {
                    // White wins by Checkmate
//                    std::cout << "Checkmate - White wins!  (" << (ply / 2) << " moves)" << std::endl;
                    return white ? 2 : 0;
                } else if(eval < -(INF - 50)) {
                    // Black wins by Checkmate
//                    std::cout << "Checkmate - Black wins!  (" << (ply / 2) << " moves)" << std::endl;
                    return white ? 0 : 2;
                }
//                std::cout << "Draw - Threefold repetition!  (" << (ply / 2) << " moves)" << std::endl;
                return 1;
            }


//            printf("%zu\n", EngineMC::bestMoves().size());
//            printf("%zu\n", EngineMC::topLevelLegalMoves().size());

//            nextMove = line.back();

            if (random.bernoulli(USE_ENGINE_BEST_MOVES_PROBABILITY)) {
                nextMove = random.randomElementOf(EngineMC::bestMoves);
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
//                std::cout << "Draw - 50 move rule!  (" << (ply / 2) << " moves)" << std::endl;
                return 1;
            }


            EngineMC::repTable.insert(Zobrist::hash(currentBoard, currentState));

            currentBoard.makeMove(currentState, nextMove);
            currentState.update(nextMove.flags);

//            if (ply % 2 == 1) {
//                fen << (1 + (ply >> 1)) << ". ";
//            }
//            fen << Utils::moveNameNotation(nextMove) << " ";
        }

//        std::cout << "Game aborted" << std::endl;
        return 1;

//        fen << "\n";
//        return fen.str();
//        throw std::runtime_error("Game simulation aborted unexpectedly");
    }
};

Utils::Random MonteCarlo::random{};


int randomPlayout(Board& board, State state, Utils::Random rand) {
    bool white = state.whiteToMove;

    // After 300 moves of simulation a game is considered a draw
    for(int ply{0}; ply < 600; ply++) {
        std::vector<Move> moveList;
        MoveListGenerator::getMoves<false>(board, state, moveList);

        if(moveList.empty()) {
            if(MoveGenerator<MoveListGenerator, false>::pd->inCheck()) {
                // Checkmate
                return state.whiteToMove == white ? 0 : 2;
            }
            return 1;
        }

//        Move move;
//        if (rand.bernoulli(USE_RANDOM_MOVE_IN_ROLLOUT)) {
//            move = rand.randomElementOf(moveList);
//        } else {
//            int bestEval;
//            move = moveList.back();
//            moveList.pop_back();
//            for(Move m: moveList) {
//                auto [nextBoard, nextState] = fork(board, state, m);
//                int eval = subjectiveEval(evaluation::evaluatePosition(nextBoard, nextState), nextState);
//                if(eval > bestEval) {
//                    bestEval = eval;
//                    move = m;
//                }
//            }
//        }

        Move move = rand.randomElementOf(moveList);
        board.makeMove(state, move);
        state.update(move.flags);
    }

    return 1;
}


struct TreeNode;
using TPT = std::shared_ptr<TreeNode>;

struct ChildrenData {
    Move move;
    TPT node;
    float score;
};

struct TreeNode {
    int wins{0};
    int total{0};
    TPT parent;
    int index;
    std::vector<ChildrenData> children;

    TreeNode(TPT& pt, int ix) : parent(pt), index(ix) {}

    ~TreeNode() {
        for(auto& [_, x, _2]: children) {
            x.reset();
        }
    }
};

class GameTree {
    Board startBoard;
    State startState;
    Utils::Random random;

    const double c = 1.414;

public:
    TPT root;

    GameTree (const Board& board, const State& state) : startBoard{board}, startState(state) {
        root = std::make_unique<TreeNode>(root, 0);
    }
    ~GameTree() {
        root.reset();
    }

    void run() {
        /// 1. SELECT
        TPT node = root;
        Board board = startBoard;
        State state = startState;

        while(!node->children.empty()) {
            ChildrenData best = node->children.front();
            for(auto& cd: node->children) {
                if (cd.node->total > 0) {
                    cd.score = static_cast<double>(cd.node->wins) / cd.node->total + c * std::sqrt(std::sqrt(node->total) / cd.node->total);
                    if(cd.score <= best.score) {
                        continue;
                    }
                } else cd.score = INFINITY;
                best = cd;
            }

            auto [move, next, _] = best;
            board.makeMove(state, move);
            state.update(move.flags);
            node = next;
        }

        /// 2. EXPAND
        if(node->total > 0) {
            currentNode = node;
            generate<MC, false>(board, state);

            if(currentNode->children.empty()) {
                // currentNode is terminal
                return;
            }

            ChildrenData child = random.randomElementOf(currentNode->children);
            board.makeMove(state, child.move);
            state.update(child.move.flags);
            node = child.node;
        }

        /// 3. SIMULATE
        int result = randomPlayout(board, state, random);
        //        int result = MonteCarlo::simulateGame(board, state, depth);


        /// 4. BACKPROPAGATE
        node->wins += result;
        node->total += 2;

        while(node != root) {
            node = node->parent;
            node->wins += result;
            node->total += 2;
        }

//            int ix = node->index;
//            float score = static_cast<double>(node->result) / node->total + c * std::sqrt(std::sqrt(node->parent->total + 2) / node->total);
//            node->children.at(ix).score = score;
//            for(unsigned int index = ix+1; index < node->children.size(); index++)
//                updateScore(node->children.at(index).node);
//            while(ix > 0 && node->children.at(ix-1).score < score) {
//                // ensure children list sorted desc by score
//                updateScore(node->children.at(ix-1).node);
//                std::swap(node->children.at(ix-1), node->children.at(ix));
//                node->children.at(ix-1).node->index = ix-1;
//                node->children.at(ix).node->index = ix;
//                ix--;
//            }
//        }

    }

    static TPT currentNode;
    class MC {
        template<State state, Piece_t piece, Flag_t flags = MoveFlag::Silent>
        static void registerMove([[maybe_unused]] const Board &board, BB from, BB to) {
            ChildrenData cd{
                createMoveFromBB(from, to, piece, flags),
                std::make_shared<TreeNode>(currentNode, currentNode->children.size()),
                0
            };
            currentNode->children.emplace_back(cd);
        }
        friend class MoveGenerator<MC, false>;
    };
};

TPT GameTree::currentNode{nullptr};


Move MCTS(const Board& board, const State state, int iterations=25000) {
    GameTree gt{board, state};
    for(int i = 0; i < iterations; i++) {
        gt.run();
    }
    ChildrenData best = *std::max_element(gt.root->children.begin(), gt.root->children.end(), [](auto& a, auto& b) {return a.score < b.score;});
    return best.move;
}

#endif //DORY_MONTE_CARLO_H
