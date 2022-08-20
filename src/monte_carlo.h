//
// Created by robin on 09.08.2022.
//

#ifndef CHESSENGINE_MONTE_CARLO_H
#define CHESSENGINE_MONTE_CARLO_H

#include <random>
#include <queue>
#include "movecollectors.h"

using BoardCollector = MoveCollectors::SuccessorBoards;

namespace RandomUtils {
    int randomNumber(int max) {
        static std::random_device rd;     // Only used once to initialise (seed) engine
        static std::mt19937 rng(rd());    // Random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> uni(0, max-1);
        return uni(rng);
    }

    ExtendedBoard& generateRandomMove(ExtendedBoard& board) {
        BoardCollector::getLegalMoves(board);
        int size = static_cast<int>(BoardCollector::positions.size());
        int index = randomNumber(size);
        return BoardCollector::positions.at(index);
    }
}

namespace MonteCarlo {

    struct GameTreeNodeMC {
        std::priority_queue<GameTreeNodeMC*> next{};
        ExtendedBoard board;
        GameTreeNodeMC* parent;
        int moves{0};
        float wins{0};

        GameTreeNodeMC(ExtendedBoard brd, GameTreeNodeMC* prt) : board{brd}, parent{prt} {}

        bool operator> (GameTreeNodeMC& a) const {
            return moves > a.moves;
        }
        bool operator< (GameTreeNodeMC& a) const {
            return moves < a.moves;
        }
    };

    class ExpanderMoveCollectorMC {
    public:
        static GameTreeNodeMC* currentNode;
        static std::vector<GameTreeNodeMC*> nodes;
        static float result;

        static void expand(GameTreeNodeMC* node) {
            currentNode = node;
            nodes.clear();
            run(node->board);
        }

        static void run(ExtendedBoard& ebrd) {
            Utils::template run<ExpanderMoveCollectorMC, 1>(ebrd.state_code, ebrd.board);
        }

        template<State state, int depth>
        static void main(Board& board) {
            MoveGenerator<ExpanderMoveCollectorMC>::template generate<state, 1>(board);
        }

        static GameTreeNodeMC* getRandomChild() {
            int index = RandomUtils::randomNumber(static_cast<int>(nodes.size()));
            return nodes.at(index);
        }

    private:
        template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
        static void registerMove([[maybe_unused]] Board &board, [[maybe_unused]] BB from, [[maybe_unused]] BB to) {}

        template<State nextState, int depth>
        static void next([[maybe_unused]] Board& nextBoard) {
            ExtendedBoard ebrd = getExtendedBoard<nextState>(nextBoard);
            auto* node = new GameTreeNodeMC{ ebrd, currentNode };
            currentNode->next.push( node );
            nodes.push_back(node);
        }

        template<bool white>
        static void endOfGame(bool isCheck) {
            if constexpr (white)
                result = isCheck ? 1 : 0.5;
            else result = isCheck ? -1 : 0.5;
        }

        friend class MoveGenerator<ExpanderMoveCollectorMC>;
    };

    GameTreeNodeMC* ExpanderMoveCollectorMC::currentNode{};
    std::vector<GameTreeNodeMC*> ExpanderMoveCollectorMC::nodes{};
    float ExpanderMoveCollectorMC::result{0};


    class TreeBuilder {
    private:
        static GameTreeNodeMC* select(GameTreeNodeMC& root) {
            GameTreeNodeMC* node{&root};
            while(!node->next.empty()) {
                node = node->next.top();
            }
            return node;
        }

        static GameTreeNodeMC* expand(GameTreeNodeMC* node) {
            ExpanderMoveCollectorMC::expand(node);
            return ExpanderMoveCollectorMC::getRandomChild();
        }

        static float simulate(GameTreeNodeMC& node) {
            // TODO: realize via custom movecollector
            // return 0 on loss, 1 on draw and 2 on win

            GameTreeNodeMC* board = &node;
            int i{0};

            while(ExpanderMoveCollectorMC::result == 0) {
                ExpanderMoveCollectorMC::expand(board);
                board = ExpanderMoveCollectorMC::getRandomChild();

                i++;
                std::cout << i << std::endl;
                Utils::print_board(board->board.board);
                std::cout << ((board->board.state_code & 0b10000) ? "white to move" : "black to move") << std::endl;
            }
            std::cout << "Simulation ended after " << i << " iterations. Result: " << ExpanderMoveCollectorMC::result << std::endl;

            return ExpanderMoveCollectorMC::result;
        }
        
        static void backpropagate(GameTreeNodeMC* leaf, float result) {
            GameTreeNodeMC* node{leaf};
            while (node != nullptr) {
                node->moves += 2;
                node->wins += result;
                node = node->parent;
            }
        }

    public:
        template<State state>
        static void generateGameTree(Board &board, int iterations) {
            ExtendedBoard brd = getExtendedBoard<state>(board);
            GameTreeNodeMC root { brd, nullptr };

            for(int it{0}; it < iterations; ++it) {
                GameTreeNodeMC* leaf = select(root);

                GameTreeNodeMC* candidate = expand(leaf);

                float result = simulate(*candidate);

                backpropagate(candidate, result);
            }

            std::cout << (root.wins / 2) << " / " << (root.moves / 2) << std::endl;
        }
    };
}

#endif //CHESSENGINE_MONTE_CARLO_H
