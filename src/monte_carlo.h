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
        std::uniform_int_distribution<int> uni(0, max);
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
        ExtendedBoard* board;
        GameTreeNodeMC* parent;
        int moves{0}, wins{0};

        GameTreeNodeMC(ExtendedBoard& brd, GameTreeNodeMC* prt) : board{&brd}, parent{prt} {}

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
        static int numMoves;

        static void expand(GameTreeNodeMC* node) {
            currentNode = node;
            numMoves = 0;
            Utils::template run<ExpanderMoveCollectorMC, 1>(node->board->state_code, node->board->board);
        }

        template<State state, int depth>
        static void main(Board& board) {
            MoveGenerator<ExpanderMoveCollectorMC>::template generate<state, 1>(board);
        }

        static GameTreeNodeMC* getRandomChild() {
            int index = RandomUtils::randomNumber(numMoves);
            return nodes.at(index);
        }

    private:
        template<State state, int depth, Piece_t piece, Flag_t flags = MoveFlag::Silent>
        static void registerMove(Board &board, [[maybe_unused]] BB from, [[maybe_unused]] BB to) {
            ExtendedBoard nextBoard = getExtendedBoard<state>(board);
            auto* node = new GameTreeNodeMC{ nextBoard, currentNode };
            currentNode->next.push( node );
            nodes.push_back(node);
            numMoves++;
        }

        template<State nextState, int depth>
        static void next([[maybe_unused]] Board& nextBoard) {}

        friend class MoveGenerator<ExpanderMoveCollectorMC>;
    };

    GameTreeNodeMC* ExpanderMoveCollectorMC::currentNode{};
    std::vector<GameTreeNodeMC*> ExpanderMoveCollectorMC::nodes{};
    int ExpanderMoveCollectorMC::numMoves{0};


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

        static int simulate(GameTreeNodeMC& node) {
            // TODO: realize via custom movecollector
            // return 0 on loss, 1 on draw and 2 on win

            int rand = RandomUtils::randomNumber(100);
            if(rand > 80) return 2;
            if(rand > 50) return 1;
            return 0;
        }
        
        static void backpropagate(GameTreeNodeMC* leaf, int result) {
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
                std::cout << "select" << std::endl;
                GameTreeNodeMC* leaf = select(root);

                std::cout << "expand" << std::endl;
                GameTreeNodeMC* candidate = expand(leaf);

                std::cout << "simulate" << std::endl;
                int result = simulate(*candidate);

                std::cout << "backpropagate" << std::endl;
                backpropagate(candidate, result);
            }

            std::cout << (root.wins / 2) << " / " << (root.moves / 2) << std::endl;
        }
    };
}

#endif //CHESSENGINE_MONTE_CARLO_H
