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

    template<State state>
    ExtendedBoard& generateRandomMove(Board& board) {
        BoardCollector::template generateGameTree<state, 1>(board);
        int size = static_cast<int>(BoardCollector::positions.size());
        int index = randomNumber(size);
        return BoardCollector::positions.at(index);
    }

    ExtendedBoard& generateRandomMove2(ExtendedBoard& board) {
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

    class TreeBuilder {
    private:
        static GameTreeNodeMC* select(GameTreeNodeMC& root) {
            GameTreeNodeMC* node{&root};
            while(!node->next.empty()) {
                node = node->next.top();
            }
            return node;
        }

        static bool simulate(GameTreeNodeMC& node) {
            // TODO: realize via custom movecollector
        }
        
        static void backpropagate(GameTreeNodeMC& leaf, bool win) {
            GameTreeNodeMC* node{&leaf};
            do {
                node->moves++;
                if(win) node->wins++;
                node = node->parent;
            } while (node != nullptr);
        }

    public:
        template<State state>
        static void generateGameTree(Board &board, int iterations) {
            GameTreeNodeMC root { getExtendedBoard<state>(board), nullptr };

            for(int it{0}; it < iterations; ++it) {
                GameTreeNodeMC* leaf = select(root);
                GameTreeNodeMC follow { RandomUtils::generateRandomMove2(*leaf->board), leaf };
                leaf->next.push(&follow);

                bool win = simulate(follow);
                backpropagate(follow, win);
            }
        }

    };
}


#endif //CHESSENGINE_MONTE_CARLO_H
